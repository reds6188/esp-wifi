#include "wi-fi.h"

WiFiHandler::WiFiHandler(const char * hostname) {
	WiFi.setHostname(hostname);

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
}

void WiFiHandler::begin(wifi_mode_t mode) {
	if(mode == WIFI_AP) {
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
	}
}

void WiFiHandler::reconnect(void) {
	WiFi.reconnect();
}

void WiFiHandler::disconnect(void) {
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

bool WiFiHandler::setCredentials(const char* ssid, const char* password)
{
	if(esp_wifi_get_config(WIFI_IF_STA, &_sta_config) == ESP_OK) {
		strncpy((char *)_sta_config.sta.ssid, ssid, 32);
		strncpy((char *)_sta_config.sta.password, password, 64);
		if(esp_wifi_set_config(WIFI_IF_STA, &_sta_config) == ESP_OK)
			return true;
		else
			console.error(WIFI_T, "Error on setting configuration");
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