#include <Arduino.h>

#define CONNECTION_TIMEOUT 15000
#define CONFIGURATION_FILENAME "/configuration.json"
#define JSON_CONFIG_BUF_SIZE (16*1024)
#define MAX_STRING_LENGTH 64

// created to omit "ADC2 is no longer supported"
struct InputHardwareAction {
  bool enabled;
  int read_pin;
  std::vector<int> hz_pins;
  std::vector<int> low_pins;
  std::vector<int> high_pins;

  float read() const {
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
    delayMicroseconds(10);
    return (float)analogRead(read_pin) / 4095.f;
  }
};

const int FAN_TURN_ON_TEMP = 70;
const int FAN_TURN_OFF_TEMP = 50;
const float THERMISTOR_CONST = 4050.0;
const float THERMISTOR_R0 = 47000;
const float THERMISTOR_IN_SERIES_RESISTOR = 47000;
const float THERMISTOR_T0 = 25.0 + 273.15;

const int RESET_CONFIGURATION_PIN = D3;
const int FAN_PIN = D4;
const int LED_GPIO_OUTPUTS[] = {D7, D8, D9, D10};

const InputHardwareAction POTENTIOMETER_HARDWARE_ACTIONS[4] = {
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D3, D4},
    .high_pins = {D2}
  },
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D4},
    .high_pins = {D2, D3}
  },
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {},
    .high_pins = {D2, D3, D4}
  },
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D3},
    .high_pins = {D2, D4}
  },
};


const InputHardwareAction THERMISTOR_HARDWARE_ACTIONS[4] = {
 {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D2, D4},
    .high_pins = {D3}
  },
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D2, D3},
    .high_pins = {D4}
  },
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D2, D3, D4},
    .high_pins = {}
  },
  {
    .enabled = true,
    .read_pin = A0,
    .hz_pins = {},
    .low_pins = {D2},
    .high_pins = {D3, D4}
  },
};