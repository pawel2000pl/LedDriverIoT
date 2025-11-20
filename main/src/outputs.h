#pragma once
#include <array>

#include "lib/ArduinoJson/ArduinoJson.h"
#include "common_types.h"
#include "json_utils.h"

std::vector<fixed32_f> toFixedpointVector(const JsonVariantConst& source);

namespace outputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void writeOutput();
    void setFadeoutScalling(fixed32_c value);
    void setColor(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w);
    void setColor(const ColorChannels& channels);
    ColorChannels getColor();
    ColorChannels getTailoredScalling();
    
}
