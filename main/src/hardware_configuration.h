#pragma once

#include <Arduino.h>
#include <vector>
#include <array>

#include "lib/ArduinoJson/ArduinoJson.h"

#include "inplace_vector.h"
#include "common_types.h"
#include "json_utils.h"

#define ANALOG_READ_MAX 4095
#define RELAXATION_DELAY 30
#define RELAXATION_PULL_DELAY 5000

namespace hardware {

	struct InputHardwareAction {
		bool enabled;
		char read_pin;
		inplace_vector<char, 4> hz_pins;
		inplace_vector<char, 4> low_pins;
		inplace_vector<char, 4> high_pins;
		fixed64 read() const;

		int getPin(int disabledValue=31) const;
		void setInput() const;
	};


	struct HardwareConfiguration {
		const char* name;
		char fanPin;
		char resetPin;
		std::array<InputHardwareAction, 4> potentiometers;
		std::array<InputHardwareAction, 4> thermistors;
		std::array<char, 4> outputs;
		
		// if pin 21 is required, IT MUST BE THE LAST ON THE LIST
		inplace_vector<char, 4> requires_potentiometers = {};
		inplace_vector<char, 4> requires_hz = {};
		inplace_vector<char, 4> requires_lo = {};
		inplace_vector<char, 4> requires_hi = {};
		inplace_vector<char, 4> requires_shorted = {};
		inplace_vector<char, 4> requires_not_shorted = {};

		bool available() const;
		void setup() const;
		String getCode() const;
	};

	void detectHardware();

	extern inplace_vector<HardwareConfiguration*, 5> configurations;
	extern HardwareConfiguration* configuration;

}


