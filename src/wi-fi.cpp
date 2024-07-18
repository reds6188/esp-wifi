#include "wi-fi.h"

WiFiHandler::WiFiHandler(const char * hostname) {
	WiFi.setHostname(hostname);
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

void WiFiHandler::onEvent(void cbOnEvent(WiFiEvent_t event, WiFiEventInfo_t info), WiFiEvent_t event) {
	WiFi.onEvent(cbOnEvent, event);
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