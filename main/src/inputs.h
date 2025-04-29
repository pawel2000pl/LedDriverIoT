#pragma once
#include <ArduinoJson.h>
#include <array>

#include "common_types.h"
#include "json_utils.h"


namespace inputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void setRGBW(fixed64 r, fixed64 g, fixed64 b, fixed64 w);
    void setHSVW(fixed64 h, fixed64 s, fixed64 v, fixed64 w);
    void setHSLW(fixed64 h, fixed64 s, fixed64 l, fixed64 w);
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
