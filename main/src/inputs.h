#pragma once
#include <ArduinoJson.h>
#include <array>

#include "common_types.h"
#include "json_utils.h"


namespace inputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void setRGBW(fraction32 r, fraction32 g, fraction32 b, fraction32 w);
    void setHSVW(fraction32 h, fraction32 s, fraction32 v, fraction32 w);
    void setHSLW(fraction32 h, fraction32 s, fraction32 l, fraction32 w);
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
