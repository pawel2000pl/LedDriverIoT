#pragma once

#include <Arduino.h>
#include "lib/ArduinoJson/ArduinoJson.h"
#include "json_utils.h"

namespace knobs {

    void updateConfiguration(const JsonVariantConst configuration);
    void check(bool force=false);
    void checkTimer();
    void init();

}
