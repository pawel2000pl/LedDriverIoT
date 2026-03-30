#pragma once

#include "lib/ArduinoJson/ArduinoJson.h"

namespace default_color {

    void updateConfiguration(const JsonVariantConst configuration);
    void setDefaultColor();

}
