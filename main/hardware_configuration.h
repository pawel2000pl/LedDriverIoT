#pragma once

#include <Arduino.h>
#include <vector>
#include <array>

#include <ArduinoJson.h>

#define ANALOG_READ_MAX 4095
#define RELAXATION_DELAY 30
#define RELAXATION_PULL_DELAY 5000

struct InputHardwareAction {
  bool enabled;
  int read_pin;
  std::vector<int> hz_pins;
  std::vector<int> low_pins;
  std::vector<int> high_pins;
  float read() const;
};


extern std::array<InputHardwareAction, 4> POTENTIOMETER_HARDWARE_ACTIONS;
extern std::array<InputHardwareAction, 4> THERMISTOR_HARDWARE_ACTIONS;
extern int FAN_PIN;

void detectHardware();
