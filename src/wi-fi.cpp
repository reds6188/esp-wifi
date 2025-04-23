#include "wi-fi.h"

WiFiHandler::WiFiHandler(const char * hostname) {
	WiFi.setHostname(hostname);
	setApSsid("DefaultApSsid");

	WiFi.onEvent(
		[](WiFiEvent_t event, WiFiEventInfo_t info) {
			console.success(WIFI_T, "EVENT - Wi-Fi ready, waiting for connection...");
		},
		ARDUINO_EVENT_WIFI_STA_START
	);

	WiFi.onEvent(
		[this](WiFiEvent_t event, WiFiEventInfo_t info) {
			console.success(WIFI_T, "EVENT - Wi-Fi connection was established with network \"" + getSSID() + "\"");
			if(!cbOnConnect)
				return;
			cbOnConnect();
		},
		ARDUINO_EVENT_WIFI_STA_CONNECTED
	);

	WiFi.onEvent(
		[this](WiFiEvent_t event, WiFiEventInfo_t info) {
			uint8_t reason = info.wifi_sta_disconnected.reason;
			console.warning(WIFI_T, "EVENT - Wi-Fi was disconnected, reason code: " + String(reason) + " (" + String(WiFi.disconnectReasonName((wifi_err_reason_t)reason)) + ")");
			reconnect();
			if(!cbOnDisconnect)
				return;
			cbOnDisconnect();
		},
		ARDUINO_EVENT_WIFI_STA_DISCONNECTED
	);

	WiFi.onEvent(
		[](WiFiEvent_t event, WiFiEventInfo_t info) {
			IPAddress local_IP = IPAddress(info.got_ip.ip_info.ip.addr);
			console.success(WIFI_T, "EVENT - Local IP address: " + local_IP.toString());
		},
		ARDUINO_EVENT_WIFI_STA_GOT_IP
	);

	WiFi.onEvent(
		[this](WiFiEvent_t event, WiFiEventInfo_t info) {
			_scanning = false;
			console.success(WIFI_T, "EVENT - Wi-Fi scan completed");
			if(!info.wifi_scan_done.status) {
				console.success(WIFI_T, "Found " + String(info.wifi_scan_done.number) + " network(s)");
				for(int16_t i=0 ; i<info.wifi_scan_done.number ; i++) {
					String encriptionType = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
					String rssi = String(WiFi.RSSI(i));
					console.info(WIFI_T, String(i) + ": " + WiFi.SSID(i) + " (" + rssi + " dBm) " + encriptionType);
				}
				console.info(WIFI_T, "Note: networks with \"*\" char are encrypted");
			}
			else
				console.error(WIFI_T, "Failed to complete scan!");
				if(WiFi.status() != WL_CONNECTED)
					reconnect();
		},
		ARDUINO_EVENT_WIFI_SCAN_DONE
	);
}

void WiFiHandler::begin(wifi_mode_t mode) {
	if(mode == WIFI_AP) {
		IPAddress localIP(ap_static_ip);
		IPAddress gatewayIP(ap_gateway_ip);
		IPAddress subnetMask(ap_subnet_mask);
		WiFi.mode(WIFI_AP);
		if(!WiFi.softAP(_ap_ssid, NULL, 1, 0, 1, false)) {
			console.error(WIFI_T, "AP failed to start!");
		}
		else {
			delay(100);
			if(!WiFi.softAPConfig(localIP, gatewayIP, subnetMask))
				console.error(WIFI_T, "AP configuration failed");
			else
			{
				console.success(WIFI_T, "AP started!");
				console.info(WIFI_T, "Network SSID: " + String(_ap_ssid));
				IPAddress IP = WiFi.softAPIP();
				console.info(WIFI_T, "AP IP address: " + IP.toString());
			}
		}
	}
	else if(mode == WIFI_STA) {
		WiFi.mode(WIFI_STA);
		WiFi.begin();
		if(esp_wifi_get_config(WIFI_IF_STA, &_sta_config) != ESP_OK) {
			console.error(WIFI_T, "Error on getting configuration");
		}
		else {
			console.info(WIFI_T, "SSID = " + String((char *)_sta_config.sta.ssid));
		}
	}
	else if(mode == WIFI_AP_STA) {
		WiFi.mode(WIFI_AP_STA);
		// Start AP -----------------------------------------------------------
		if(!WiFi.softAP(_ap_ssid, NULL, 1, 0, 1, false)) {
			console.error(WIFI_T, "AP failed to start!");
		}
		else {
			delay(100);
			if(!WiFi.softAPConfig(localIP, gatewayIP, subnetMask))
				console.error(WIFI_T, "AP configuration failed");
			else
			{
				console.success(WIFI_T, "AP started!");
				console.info(WIFI_T, "AP SSID: " + String(_ap_ssid));
				IPAddress IP = WiFi.softAPIP();
				console.info(WIFI_T, "AP IP address: " + IP.toString());
			}
		}
		// Start STA ----------------------------------------------------------
		WiFi.begin();
		if(esp_wifi_get_config(WIFI_IF_STA, &_sta_config) != ESP_OK) {
			console.error(WIFI_T, "Error on getting configuration");
		}
		else {
			console.info(WIFI_T, "STA SSID: " + String((char *)_sta_config.sta.ssid));
		}
	}
}

void WiFiHandler::end(void) {
	WiFi.mode(WIFI_MODE_NULL);
}

void WiFiHandler::reconnect(void) {
	console.info(WIFI_T, "Reconnecting...");
	WiFi.reconnect();
}

void WiFiHandler::disconnect(void) {
	console.warning(WIFI_T, "Disconnecting...");
	WiFi.disconnect();
}

void WiFiHandler::onEvent(void cbOnEvent(WiFiEvent_t event, WiFiEventInfo_t info), WiFiEvent_t event) {
	WiFi.onEvent(cbOnEvent, event);
}

void WiFiHandler::onConnect(void (*callback)(void)) {
	cbOnConnect = callback;
}

void WiFiHandler::onDisconnect(void (*callback)(void)) {
	cbOnDisconnect = callback;
}

void WiFiHandler::setApSsid(const char* ssid) {
	strncpy(_ap_ssid, ssid, 32);
}

bool WiFiHandler::setCredentials(const char* ssid, const char* password)
{
	if(esp_wifi_get_config(WIFI_IF_STA, &_sta_config) == ESP_OK) {
		strncpy((char *)_sta_config.sta.ssid, ssid, 32);
		strncpy((char *)_sta_config.sta.password, password, 64);
		if((getMode() == WIFI_MODE_STA) || (getMode() == WIFI_MODE_AP_STA)) {
			if(esp_wifi_set_config(WIFI_IF_STA, &_sta_config) == ESP_OK)
				return true;
			else
				console.error(WIFI_T, "Error on setting configuration");
		}
	}
	else
		console.error(WIFI_T, "Error on getting configuration");

	return false;
}

String WiFiHandler::getLocalIP(void) {
	IPAddress IP = WiFi.localIP();
	return IP.toString();
}

String WiFiHandler::getSSID(void) {
	if(esp_wifi_get_config(WIFI_IF_STA, &_sta_config) == ESP_OK) {
		return String((char *)_sta_config.sta.ssid);
	}
	return String();
}

int8_t WiFiHandler::getRSSI(void) {
	return WiFi.RSSI();
}

String WiFiHandler::getMacAddress(wifi_interface_t interface) {
	uint8_t mac[6];
	char macStr[18] = { 0 };
	if(esp_wifi_get_mac(interface, mac) == ESP_OK)
		sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return String(macStr);
}

void WiFiHandler::printMacAddress(wifi_interface_t interface) {

	if(interface == WIFI_IF_STA)
		console.info(WIFI_T, "Wi-Fi MAC Address (STA): " + getMacAddress(interface));
	else if(interface == WIFI_IF_AP)
		console.info(WIFI_T, "Wi-Fi MAC Address (STA): " + getMacAddress(interface));
	else
		console.error(WIFI_T, "Unknown Wi-Fi interface");
}

void WiFiHandler::startScanNetworks(void) {
	wl_status_t status = WiFi.status();
	console.info(WIFI_T, "Connection status = " + String(status));
	if((status != WL_IDLE_STATUS) && (status != WL_CONNECTED))
		disconnect();
	int16_t res = WiFi.scanNetworks(true, false, false, 300, 0, NULL, NULL);
	if(res == WIFI_SCAN_RUNNING ) {
		console.success(WIFI_T, "Scan is running");
		_scanning = true;
		return;
	}
	else if(res == WIFI_SCAN_FAILED) {
		console.error(WIFI_T, "Failed to start running");
	}
	else {
		console.success(WIFI_T, "Scan completed");
	}
	_scanning = true;
}