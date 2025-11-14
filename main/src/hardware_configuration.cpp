#include "hardware_configuration.h"

#include <cstdint>

namespace hardware {
	const int CLOCK_32_PINS[] = {0, 1};
	DetectedHardware detectedHardware;
}

const hardware::DetectedHardware& hardware_configuration = hardware::detectedHardware;


namespace hardware {
	const PinSets availableConfiguration = {
		.analogReadMain = 2,
		.thermistorChecker = 3,
		.analogReadSecondary = {2, 3, 4},
		.analogSelect = {6, 7, 21},
		.fanPinMain = 4,
		.fanPinAlt = 21,
		.altResetPin = 10,
		.outputs = {20, 8, 9, 10},
		.resetPin = 5,
		.outputsAlt = {5, 6, 7, 8},
		.resetPinAlt = 9
	};


	fixed64 avgAnalog(int pin, unsigned count) {
		std::uint64_t sum = 0;
		for (unsigned i=0;i<count;i++)
			sum += analogRead(pin);
		return fixed64::fraction(sum, count);
	}


	fixed32_c InputHardwareAction::read() const {
			if (!enabled) return 0;
			for (auto& pin : hz_pins)
					pinMode(pin, INPUT);
			for (auto& pin : high_pins) {
					pinMode(pin, OUTPUT);
					digitalWrite(pin, HIGH);
			}
			for (auto& pin : low_pins) {
					pinMode(pin, OUTPUT);
					digitalWrite(pin, LOW);
			}
			pinMode(read_pin, INPUT);
			delayMicroseconds(RELAXATION_DELAY);
			return avgAnalog(read_pin, 5) * fixed64::fraction(1, ANALOG_READ_MAX);
	}


	bool analogHasPotentiometer(int pin) {
		pinMode(pin, INPUT_PULLDOWN);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v1 = analogRead(pin);
		pinMode(pin, INPUT_PULLUP);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v2 = analogRead(pin);
		return (v1 > ANALOG_READ_MAX * 0.3f) || (v2 < ANALOG_READ_MAX * 0.7f);
	}


	int InputHardwareAction::getPin(int disabledValue) const {
		return enabled ? read_pin : disabledValue;
	}


	String DetectedHardware::getCode() const {
		const char charset[] = "0123456789abcdefghijklmnopqrstu#";
		int numbers[] = {
			fanPin, resetPin,
			potentiometers[0].getPin(), potentiometers[1].getPin(), potentiometers[2].getPin(), potentiometers[3].getPin(), 
			thermistors[0].getPin(), thermistors[1].getPin(), thermistors[2].getPin(), thermistors[3].getPin(), 
			outputs[0], outputs[1], outputs[2], outputs[3],
			-1
		};
		char buf[64];
		int i = -1;
		while (numbers[++i] >= 0)
			buf[i] = charset[numbers[i] & 31];
		buf[i] = 0;
		return String(buf);
	}


	bool pinsAreConnected(int a, int b) {
		pinMode(a, INPUT_PULLUP);
		pinMode(b, INPUT_PULLUP);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		bool v1a = !digitalRead(a);
		bool v1b = !digitalRead(b);

		pinMode(a, INPUT_PULLDOWN);
		pinMode(b, INPUT_PULLDOWN);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		bool v2a = digitalRead(a);
		bool v2b = digitalRead(b);

		bool result = false;

		if (v1a && v2a) { // if pin a has HZ
			pinMode(a, OUTPUT);
			digitalWrite(a, LOW);
			delayMicroseconds(RELAXATION_PULL_DELAY);
			result = digitalRead(b);
		} else if (v1b && v2b) { // if pin b has HZ
			pinMode(b, OUTPUT);
			digitalWrite(b, LOW);
			delayMicroseconds(RELAXATION_PULL_DELAY);
			result = digitalRead(a);
		}

		pinMode(a, INPUT);
		pinMode(b, INPUT);

		return result;
	}


	bool pinsAreDisconnected(int a, int b) {
		return !pinsAreConnected(a, b);
	}


	bool PinSets::multiplexerAvailable() const {
		return pinsAreDisconnected(analogSelect[0], analogSelect[1])
			&& pinsAreDisconnected(analogSelect[1], analogSelect[2])
			&& pinsAreDisconnected(analogSelect[0], analogSelect[2])
			&& pinsAreDisconnected(altOutputsPin, analogSelect[2]);
	}


	void PinSets::setAnalog(int x) const {
		for (unsigned i=0;i<3;i++) {
			pinMode(analogSelect[i], OUTPUT);
			digitalWrite(analogSelect[i], (x & (1 << i)) ? HIGH : LOW);
		}
		delayMicroseconds(RELAXATION_DELAY);
	}


	std::vector<int> PinSets::getAnalogSelectPins(int x, bool value) const {
		std::vector<int> result;
		result.reserve(4);
		for (unsigned i=0;i<3;i++) 
			if (value == !!(x & (1 << i)))
				result.push_back(analogSelect[i]);
		return result;
	}


	DetectedHardware PinSets::detect() const {
		DetectedHardware result;

		if (multiplexerAvailable()) {
			unsigned x = 0;
			result.fanPin = fanPinMain;
			pinMode(thermistorChecker, INPUT_PULLUP);
			delayMicroseconds(RELAXATION_PULL_DELAY);
			bool thermistorsAvailable = digitalRead(thermistorChecker) == HIGH;

			for (unsigned i=0;i<4;i++) {
				setAnalog(x);
				result.thermistors[i] = (InputHardwareAction){
					.enabled = analogHasPotentiometer(analogReadMain),
					.read_pin = analogReadMain,
					.hz_pins = {},
					.low_pins = getAnalogSelectPins(x, false),
					.high_pins = getAnalogSelectPins(x, true)
				};
				if(thermistorsAvailable) x++;
			}

			pinMode(thermistorChecker, INPUT_PULLDOWN);
			delayMicroseconds(RELAXATION_PULL_DELAY);
			bool potentiometersAvailable = digitalRead(thermistorChecker) == LOW;

			for (unsigned i=0;i<4;i++) {
				setAnalog(x);
				result.potentiometers[i] = (InputHardwareAction){
					.enabled = potentiometersAvailable && analogHasPotentiometer(analogReadMain),
					.read_pin = analogReadMain,
					.hz_pins = {},
					.low_pins = getAnalogSelectPins(x, false),
					.high_pins = getAnalogSelectPins(x, true)
				};
				if (potentiometersAvailable) x++;
			}

			pinMode(thermistorChecker, INPUT);
			setAnalog(0);

		} else {
			result.fanPin = fanPinAlt;

			unsigned analogReadSecondarySize = analogReadSecondary.size();
			if (analogReadSecondarySize > 4) analogReadSecondarySize = 4;
			for (unsigned i=0;i<analogReadSecondarySize;i++) {
				result.potentiometers[i] = (InputHardwareAction){
					.enabled = analogHasPotentiometer(analogReadMain),
					.read_pin = analogReadSecondary[i],
					.hz_pins = {},
					.low_pins = {},
					.high_pins = {}
				};    
			}

			for (unsigned i=analogReadSecondarySize;i<4;i++)
				result.potentiometers[i].enabled = false;
			for (unsigned i=0;i<4;i++)
				result.thermistors[i].enabled = false;
			for (unsigned i=0;i<3;i++)
				pinMode(analogSelect[i], INPUT);
		}

		if (pinsAreConnected(altOutputsPin, result.fanPin)) { // reset && outputs alt
			result.resetPin = resetPinAlt;
			for (unsigned i=0;i<4;i++)
				result.outputs[i] = outputsAlt[i];
		} else { // reset && outputs main
			result.resetPin = resetPin;
			for (unsigned i=0;i<4;i++)
				result.outputs[i] = outputs[i];
		}

		pinMode(result.resetPin, INPUT_PULLUP);
		pinMode(result.fanPin, OUTPUT);
		digitalWrite(result.fanPin, LOW);
		delayMicroseconds(RELAXATION_DELAY);

		return result;
	}


	void detectHardware() {
		analogReadResolution(12);
		detectedHardware = availableConfiguration.detect();
	}

}
