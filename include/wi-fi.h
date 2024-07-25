#ifndef WI_FI_H_
#define WI_FI_H_

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <console.h>

#define WIFI_T      "WI-FI"

#define AP_STATIC_IP    { 192, 168, 4, 1 }
#define AP_GATEWAY_IP   { 192, 168, 4, 0 }
#define AP_SUBNET_MASK  { 192, 168, 4, 0 }

class WiFiHandler {
	private:
		wifi_config_t _sta_config;
		wifi_config_t _ap_config;
		bool _scanning;
		void (*cbOnConnect)(void);
		void (*cbOnDisconnect)(void);
	public:
		WiFiHandler(const char * hostname);
		void begin(wifi_mode_t mode);
		void reconnect(void);
		void disconnect(void);
		void onEvent(void cbOnEvent(WiFiEvent_t event, WiFiEventInfo_t info), WiFiEvent_t event);
		void onConnect(void (*callback)(void));
		void onDisconnect(void (*callback)(void));
		bool setCredentials(const char* ssid, const char* password);
		String getLocalIP(void);
		String getSSID(void);
		void startScanNetworks(void);
};

#endif  /* WI_FI_H_ */