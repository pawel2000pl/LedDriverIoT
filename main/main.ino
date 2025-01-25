#include <Arduino.h>

#include "src/wifi.h"
#include "src/knobs.h"
#include "src/server.h"
#include "src/modules.h"
#include "src/ledc_driver.h"
#include "src/temperature.h"
#include "src/configuration.h"
#include "src/hardware_configuration.h"
#include "src/endpoints/initialization.h"

#define RESET_CONF_TIME 10000
#define RARE_CHECKS_INTERVAL (15000)

unsigned long long int reset_timer = 0;


void resetConfiguration() {
	configuration::resetConfiguration();
	modules::updateModules();
}


void checkReset() {
	if (digitalRead(hardware_configuration.resetPin) == 0) {
		if (reset_timer == 0)
			reset_timer = millis();
		else if (millis() - reset_timer >= RESET_CONF_TIME) {
			configuration::resetConfiguration();
			ESP.restart();
		}
	} else 
		reset_timer = 0;
}


void setup() {
	Serial.begin(115200);  
	Serial.println("Initialization");
	randomSeed(29615);
	hardware::detectHardware();
	configuration::init();
	temperature::init();
	initLedC();
	modules::updateModules();
	wifi::fastInit();
	knobs::check(true);
	knobs::attachTimer();
	endpoints::configureServer();
	knobs::setDefaultColor();
}

unsigned long long int rareChecksTime = 0;

void loop() {

	if (wifi::connected()) {
		unsigned long long int ts = 0;
		unsigned i = 0;
		do {
			unsigned long long int t = millis();
			server::loop();
			ts = millis() - t;
		} while (ts > 1 && i++ < 100);
	}

	modules::execTaskQueue();
	checkReset();
	
	if (rareChecksTime == 0 || millis() - rareChecksTime > RARE_CHECKS_INTERVAL) {
		wifi::checkConnection();
		temperature::check();
		rareChecksTime = millis();
	}

}
