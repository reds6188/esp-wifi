#ifndef WI_FI_H_
#define WI_FI_H_

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <console.h>

#define WIFI_T      "WI-FI"

static const uint8_t ap_static_ip[] = { 192, 168, 4, 1 };
static const uint8_t ap_gateway_ip[] = { 192, 168, 4, 0 };
static const uint8_t ap_subnet_mask[] = { 255, 255, 255, 0 };

/*
#define AP_STATIC_IP    { 192, 168, 4, 1 }
#define AP_GATEWAY_IP   { 192, 168, 4, 0 }
#define AP_SUBNET_MASK  { 192, 168, 4, 0 }
*/

class WiFiHandler {
	private:
		wifi_config_t _sta_config;
		wifi_config_t _ap_config;
		char _ap_ssid[32];
		bool _scanning;
		void (*cbOnConnect)(void);
		void (*cbOnDisconnect)(void);
	public:
		WiFiHandler(const char * hostname);
		void begin(wifi_mode_t mode);
		void end(void);
		void reconnect(void);
		void disconnect(void);
		void onEvent(void cbOnEvent(WiFiEvent_t event, WiFiEventInfo_t info), WiFiEvent_t event);
		void onConnect(void (*callback)(void));
		void onDisconnect(void (*callback)(void));
		void setApSsid(const char* ssid);
		bool setCredentials(const char* ssid, const char* password);
		String getLocalIP(void);
		String getSSID(void);
		void startScanNetworks(void);
};

#endif  /* WI_FI_H_ */