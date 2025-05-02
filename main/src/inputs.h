#pragma once
#include <ArduinoJson.h>
#include <array>

#include "common_types.h"
#include "json_utils.h"


namespace inputs {

    void updateConfiguration(const JsonVariantConst& configuration);
    void setRGBW(fixed32 r, fixed32 g, fixed32 b, fixed32 w);
    void setHSVW(fixed32 h, fixed32 s, fixed32 v, fixed32 w);
    void setHSLW(fixed32 h, fixed32 s, fixed32 l, fixed32 w);
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
