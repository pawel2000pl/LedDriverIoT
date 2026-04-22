#pragma once
#include <array>
#include <vector>

#include "lib/ArduinoJson/ArduinoJson.h"
#include "common_types.h"
#include "json_utils.h"

std::vector<float> toFloatVector(const JsonVariantConst source);

namespace outputs {

    void updateConfiguration(const JsonVariantConst configuration);
    void writeOutput();
    void setFadeoutScalling(fixed32_c value);
    void setColor(const ColorChannels& channels);
    ColorChannels getColor();
    ColorChannels getTailoredScalling();

}
