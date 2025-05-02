#pragma once
#include <ArduinoJson.h>
#include <array>

#include "common_types.h"
#include "json_utils.h"

std::vector<fixed32> toFloatVector(const JsonVariantConst& source);

namespace outputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void writeOutput();
    void setColor(fixed32 h, fixed32 s, fixed32 v, fixed32 w);
    void setColor(const ColorChannels& channels);
    ColorChannels getColor();
    ColorChannels getTailoredScalling();
    
}
