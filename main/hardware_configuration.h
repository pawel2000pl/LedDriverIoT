#pragma once

#include <Arduino.h>
#include <vector>
#include <array>

#include <ArduinoJson.h>

#define ANALOG_READ_MAX 4095
#define RELAXATION_DELAY 30
#define RELAXATION_PULL_DELAY 5000

namespace hardware {

	struct InputHardwareAction {
		bool enabled;
		int read_pin;
		std::vector<int> hz_pins;
		std::vector<int> low_pins;
		std::vector<int> high_pins;
		float read() const;

		int getPin(int disabledValue=31) const;
	};


	struct DetectedHardware {
		int fanPin;
		int resetPin;
		std::array<InputHardwareAction, 4> potentiometers;
		std::array<InputHardwareAction, 4> thermistors;
		std::array<int, 4> outputs;

		String getCode() const;
	};


	struct PinSets {
		int analogReadMain;
		int thermistorChecker;
		std::vector<int> analogReadSecondary;
		std::vector<int> analogSelect;
		int fanPinMain;
		int fanPinAlt;
		std::vector<int> outputs;
		int resetPin;
		
		bool multiplexerAvailable() const;
		void setAnalog(int x) const;
		std::vector<int> getAnalogSelectPins(int x, bool value) const;
		DetectedHardware detect() const;
	};


	void detectHardware();

}


const extern hardware::DetectedHardware& hardware_configuration;
