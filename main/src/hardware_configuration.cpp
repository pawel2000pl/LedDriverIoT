#include "hardware_configuration.h"
#include "logs.h"
#include <cstdint>

#include "hardware_configurations/multiplexer1.h"
#include "hardware_configurations/multiplexer2.h"
#include "hardware_configurations/multiplexer3.h"
#include "hardware_configurations/simple1.h"
#include "hardware_configurations/simple2.h"

namespace hardware {
			
	HardwareConfiguration* configurations[] = {
		&simple1,
		&simple2,
		&multiplexer1,
		&multiplexer2,
		&multiplexer3
	};


	fixed64 avgAnalog(int pin, unsigned count) {
		std::uint64_t sum = 0;
		for (unsigned i=0;i<count;i++)
			sum += analogRead(pin);
		return fixed64::fraction(sum, count);
	}


	fixed64 InputHardwareAction::read() const {
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
			auto wakeup = micros() + RELAXATION_DELAY;
			do vTaskDelay(0); while (micros() < wakeup);
			return avgAnalog(read_pin, 5) * fixed64::fraction(1, ANALOG_READ_MAX);
	}


	void InputHardwareAction::setInput() const {
		if (enabled) pinMode(read_pin, INPUT);
	}


	int InputHardwareAction::getPin(int disabledValue) const {
		return enabled ? read_pin : disabledValue;
	}


	// it just checks if a pin is not h/z
	bool analogHasPotentiometer(int pin) {
		pinMode(pin, INPUT_PULLDOWN);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v1 = analogRead(pin);
		pinMode(pin, INPUT_PULLUP);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v2 = analogRead(pin);
		return (v1 > ANALOG_READ_MAX * 0.3f) || (v2 < ANALOG_READ_MAX * 0.7f);
	}


	bool hasHz(int pin) {
		bool analogPin = pin >= 2 && pin <= 4;
		pinMode(pin, INPUT_PULLDOWN);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v1 = analogPin ? analogRead(pin) : (ANALOG_READ_MAX * digitalRead(pin));
		pinMode(pin, INPUT_PULLUP);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v2 = analogPin ? analogRead(pin) : (ANALOG_READ_MAX * digitalRead(pin));
		return (v1 < ANALOG_READ_MAX * 0.1f) || (v2 > ANALOG_READ_MAX * 0.9f);
	}


	bool hasLow(int pin) {
		bool analogPin = pin >= 2 && pin <= 4;
		pinMode(pin, INPUT_PULLDOWN);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v1 = analogPin ? analogRead(pin) : (ANALOG_READ_MAX * digitalRead(pin));
		pinMode(pin, INPUT_PULLUP);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v2 = analogPin ? analogRead(pin) : (ANALOG_READ_MAX * digitalRead(pin));
		return (v1 < ANALOG_READ_MAX * 0.1f) || (v2 < ANALOG_READ_MAX * 0.1f);
	}


	bool hasHigh(int pin) {
		bool analogPin = pin >= 2 && pin <= 4;
		pinMode(pin, INPUT_PULLDOWN);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v1 = analogPin ? analogRead(pin) : (ANALOG_READ_MAX * digitalRead(pin));
		pinMode(pin, INPUT_PULLUP);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		unsigned v2 = analogPin ? analogRead(pin) : (ANALOG_READ_MAX * digitalRead(pin));
		return (v1 > ANALOG_READ_MAX * 0.9f) || (v2 > ANALOG_READ_MAX * 0.9f);
	}


	bool pinsAreConnected(int a, int b) {
		if (a == b) return true;

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


	bool HardwareConfiguration::available() const {
		for (auto pin : requires_potentiometers)
			if (!analogHasPotentiometer(pin)) return false;
		for (auto pin : requires_hz)
			if (!hasHz(pin)) return false;
		for (auto pin : requires_lo)
			if (!hasLow(pin)) return false;
		for (auto pin : requires_hi)
			if (!hasHigh(pin)) return false;
		int shorted_size = requires_shorted.size();
		for (int i=0;i<shorted_size;i++)
			pinMode(requires_shorted[i], INPUT);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		for (int i=1;i<shorted_size;i++) 
			if (!pinsAreConnected(requires_shorted[i-1], requires_shorted[i])) 
				return false;
		int not_shorted_size = requires_not_shorted.size();
		for (int i=0;i<not_shorted_size;i++)
			pinMode(requires_not_shorted[i], INPUT);
		delayMicroseconds(RELAXATION_PULL_DELAY);
		for (int i=1;i<not_shorted_size;i++) 
			if (pinsAreConnected(requires_not_shorted[i-1], requires_not_shorted[i])) 
				return false;
		return true;
	}

	void HardwareConfiguration::setup() const {
		pinMode(fanPin, OUTPUT);
		pinMode(resetPin, INPUT_PULLUP);
		for (const auto& pin: potentiometers)
			pin.setInput();
		for (const auto& pin: thermistors)
			pin.setInput();
	}


	String HardwareConfiguration::getCode() const {
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

	
	void detectHardware() {
		analogReadResolution(12);

		unsigned size = std::end(configurations) - std::begin(configurations);

		int available = -1;
		for (int i=0;i<size;i++) {
			if (configurations[i]->available()) {
				if (available < 0) available = i;
				else {
					logs::logger.print("Error: detected at least two possible configurations: ");
					logs::logger.print(configurations[available]->name);
					logs::logger.print(" and ");
					logs::logger.println(configurations[i]->name);
				}
			}
		}

		if (available < 0) available = 0;

		logs::logger.print("Chosen configuration: ");
		logs::logger.println(configurations[available]->name);

		configurations[available]->setup();
		hardware_configuration = configurations[available];
	}

}

hardware::HardwareConfiguration* hardware_configuration;

