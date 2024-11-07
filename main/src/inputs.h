#pragma once
#include <ArduinoJson.h>
#include <array>

using ColorChannels = std::array<float, 4>;

namespace inputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void setRGBW(float r, float g, float b, float w);
    void setHSVW(float h, float s, float v, float w);
    void setHSLW(float h, float s, float l, float w);
    void setAuto(const String& colorspace, const ColorChannels& color);

    ColorChannels getRGBW();
    ColorChannels getHSVW();
    ColorChannels getHSLW();
    ColorChannels getAuto(const String& colorspace);

    String dumpFavoriteColor(const bool useWhite);
    ColorChannels decodeFavoriteColor(const String& formattedColor, bool* useWhitePtr);
    ColorChannels favoriteColorPreview(const String& colorspace, const String& formattedColor);
    void applyFavoriteColor(const String& formattedColor);

}
