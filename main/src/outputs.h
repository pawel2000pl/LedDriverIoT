#pragma once
#include <ArduinoJson.h>
#include <array>

#include "common_types.h"
#include "json_utils.h"

std::vector<fraction32> toFloatVector(const JsonVariantConst& source);

namespace outputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void writeOutput();
    void setColor(fraction32 h, fraction32 s, fraction32 v, fraction32 w);
    void setColor(const ColorChannels& channels);
    ColorChannels getColor();
    ColorChannels getTailoredScalling();
    
}
