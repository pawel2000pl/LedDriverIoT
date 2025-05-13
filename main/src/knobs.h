#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "json_utils.h"

namespace knobs {
    
    void setLock(bool lockState);
    void updateConfiguration(const JsonVariantConst& configuration);
    void setDefaultColor();
    void turnOff();
    void check(bool force=false);
    void attachTimer();

}
