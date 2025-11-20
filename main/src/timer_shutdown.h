#pragma once

#include <cstdint>
#include "lib/ArduinoJson/ArduinoJson.h"
#include "json_utils.h"

namespace timer_shutdown {

    void checkTimer();
    void resetTimer();
    void updateConfiguration(const JsonVariantConst& configuration);

}
