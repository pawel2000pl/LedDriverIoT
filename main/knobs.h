#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

namespace knobs {
    
    void updateConfiguration(const JsonVariantConst& configuration);
    void turnOff();
    void check(bool force=false);
    void attachTimer();

}
