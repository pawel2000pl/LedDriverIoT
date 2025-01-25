#pragma once
#include <ArduinoJson.h>
#include <array>

using ColorChannels = std::array<float, 4>;

std::vector<float> toFloatVector(const JsonVariantConst& source);

namespace outputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void writeOutput();
    void setColor(float h, float s, float v, float w);
    void setColor(const ColorChannels& channels);
    ColorChannels getColor();
    ColorChannels getTailoredScalling();
    
}
