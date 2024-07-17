#ifndef WI_FI_H_
#define WI_FI_H_

#include <Arduino.h>
#include <WiFi.h>

class WiFiHandler {
	private:
		wifi_config_t config;
	public:
		WiFiHandler(void);
		void begin(wifi_mode_t mode);
};

#endif  /* WI_FI_H_ */