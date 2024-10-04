#include <esp32-hal-gpio.h>
#include "hardware_configuration.h"

const int ANALOG_READ_MAIN = A0;
const int THERIMSTOR_CHECKER = A1;
const int ANALOG_READ_SECONDARY[] = {A0, A1, A2};
const int ANALOG_SELECT[] = {D4, D5, D6};
const int FAN_PIN_MAIN = D2;
const int FAN_PIN_ALT = D4;


float InputHardwareAction::read() const {
    if (!enabled) return 0.f;
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
    return (float)analogRead(read_pin) / float(ANALOG_READ_MAX);
}


std::array<InputHardwareAction, 4> POTENTIOMETER_HARDWARE_ACTIONS;
std::array<InputHardwareAction, 4> THERMISTOR_HARDWARE_ACTIONS;
int FAN_PIN;


bool analogHasPotentiometer(int pin) {
  pinMode(pin, INPUT_PULLDOWN);
  delayMicroseconds(RELAXATION_PULL_DELAY);
  unsigned v1 = analogRead(pin);
  pinMode(pin, INPUT_PULLUP);
  delayMicroseconds(RELAXATION_PULL_DELAY);
  unsigned v2 = analogRead(pin);
  return (v1 > ANALOG_READ_MAX * 0.3f) || (v2 < ANALOG_READ_MAX * 0.7f);
}


bool multiplexerAvailable() {
  pinMode(ANALOG_SELECT[0], INPUT_PULLUP);
  pinMode(ANALOG_SELECT[1], INPUT_PULLDOWN);
  pinMode(ANALOG_SELECT[2], OUTPUT);
  digitalWrite(ANALOG_SELECT[2], HIGH);
  delayMicroseconds(RELAXATION_PULL_DELAY);
  int v1 = digitalRead(ANALOG_SELECT[0]);
  int v2 = digitalRead(ANALOG_SELECT[1]);
  digitalWrite(ANALOG_SELECT[2], LOW);
  delayMicroseconds(RELAXATION_PULL_DELAY);
  int v3 = !digitalRead(ANALOG_SELECT[0]);
  int v4 = !digitalRead(ANALOG_SELECT[1]);
  return !(v1 && v2 && v3 && v4);
}


void setAnalog(int x) {
  for (unsigned i=0;i<3;i++) {
    pinMode(ANALOG_SELECT[i], OUTPUT);
    digitalWrite(ANALOG_SELECT[i], (x & (1 << i)) ? HIGH : LOW);
  }
  delayMicroseconds(RELAXATION_DELAY);
}


std::vector<int> getAnalogSelectPins(int x, bool value) {
  std::vector<int> result;
  result.reserve(4);
  for (unsigned i=0;i<3;i++) 
    if (value == !!(x & (1 << i)))
      result.push_back(ANALOG_SELECT[i]);
  return result;
}


void detectHardware() {

  analogReadResolution(12);

  if (multiplexerAvailable()) {
    unsigned x = 0;
    FAN_PIN = FAN_PIN_MAIN;
    pinMode(THERIMSTOR_CHECKER, INPUT_PULLUP);
    delayMicroseconds(RELAXATION_PULL_DELAY);
    bool thermistorsAvailable = digitalRead(THERIMSTOR_CHECKER) == HIGH;

    for (unsigned i=0;i<4;i++) {
      setAnalog(x);
      THERMISTOR_HARDWARE_ACTIONS[i] = (InputHardwareAction){
        .enabled = analogHasPotentiometer(ANALOG_READ_MAIN),
        .read_pin = ANALOG_READ_MAIN,
        .hz_pins = {},
        .low_pins = getAnalogSelectPins(x, false),
        .high_pins = getAnalogSelectPins(x, true)
      };
      if(thermistorsAvailable) x++;
    }

    pinMode(THERIMSTOR_CHECKER, INPUT_PULLDOWN);
    delayMicroseconds(RELAXATION_PULL_DELAY);
    bool potentiometersAvailable = digitalRead(THERIMSTOR_CHECKER) == LOW;

    for (unsigned i=0;i<4;i++) {
      setAnalog(x);
      POTENTIOMETER_HARDWARE_ACTIONS[i] = (InputHardwareAction){
        .enabled = potentiometersAvailable && analogHasPotentiometer(ANALOG_READ_MAIN),
        .read_pin = ANALOG_READ_MAIN,
        .hz_pins = {},
        .low_pins = getAnalogSelectPins(x, false),
        .high_pins = getAnalogSelectPins(x, true)
      };
      if (potentiometersAvailable) x++;
    }

    pinMode(THERIMSTOR_CHECKER, INPUT);
    setAnalog(0);

  } else {
    FAN_PIN = FAN_PIN_ALT;

    for (unsigned i=0;i<3;i++) {
      POTENTIOMETER_HARDWARE_ACTIONS[i] = (InputHardwareAction){
        .enabled = analogHasPotentiometer(ANALOG_READ_MAIN),
        .read_pin = ANALOG_READ_SECONDARY[i],
        .hz_pins = {},
        .low_pins = {},
        .high_pins = {}
      };    
    }

    POTENTIOMETER_HARDWARE_ACTIONS[3].enabled = false;
    for (unsigned i=0;i<4;i++)
      THERMISTOR_HARDWARE_ACTIONS[i].enabled = false;
    for (unsigned i=0;i<3;i++)
      pinMode(ANALOG_SELECT[i], INPUT);
  }


  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);
  delayMicroseconds(RELAXATION_DELAY);
}


