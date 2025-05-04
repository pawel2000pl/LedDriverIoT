#pragma once
#include <ArduinoJson.h>
#include <array>

#include "common_types.h"
#include "json_utils.h"

std::vector<fixed64> toFixedpointVector(const JsonVariantConst& source);

namespace outputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void writeOutput();
    void setColor(fixed64 h, fixed64 s, fixed64 v, fixed64 w);
    void setColor(const ColorChannels& channels);
    ColorChannels getColor();
    ColorChannels getTailoredScalling();
    
}
