#include <Arduino.h>

#include "src/logs.h"
#include "src/wifi.h"
#include "src/knobs.h"
#include "src/server.h"
#include "src/modules.h"
#include "src/threads_mgr.h"
#include "src/ledc_driver.h"
#include "src/temperature.h"
#include "src/configuration.h"
#include "src/default_color.h"
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
	delay(50); // await for stable voltage
	logs::logger.println("Initialization");
	hardware::detectHardware();
	temperature::init();
	temperature::block_until_is_ok();
	configuration::init();
	ledc::init();
	modules::updateModules();
	wifi::fastInit();
	knobs::init();
	threads_mgr::attachTimer();
	endpoints::configureServer();
	default_color::setDefaultColor();
}

unsigned long long int rareChecksTime = 0;

void loop() {

	if (wifi::connected()) {
		for (int i=0;i<100;i++) {
			server::loop();
			if (!server::resetQueryFlag()) break;
		}
	}

	modules::execTaskQueue();
	checkReset();
	
	if (rareChecksTime == 0 || millis() - rareChecksTime > RARE_CHECKS_INTERVAL) {
		wifi::checkConnection();
		if (temperature::check().tooHot()) ESP.restart();
		rareChecksTime = millis();
	}

	vTaskDelay(20 / portTICK_PERIOD_MS);
}

