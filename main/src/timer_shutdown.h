#pragma once

#include <cstdint>
#include "lib/ArduinoJson/ArduinoJson.h"
#include "json_utils.h"

namespace timer_shutdown {

    extern bool fading_out;

    void checkTimer();
    void resetTimer();
    void updateConfiguration(const JsonVariantConst configuration);

}
