#pragma once
#include <Arduino.h>
#include "lib/ArduinoJson/ArduinoJson.h"

#include "common_types.h"
#include "json_utils.h"


namespace inputs {

    enum SourceControl {scKnobs, scWeb, scFadingOut, scDefault, scAnimation};
    extern SourceControl source_control;

    void updateConfiguration(const JsonVariantConst& configuration);
    void setRGBW(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c w);
    void setHSVW(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w);
    void setHSLW(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c w);
    void setAuto(const String& colorspace, const ColorChannels& color);

    ColorChannels prepareRGBW(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c w);
    ColorChannels prepareHSVW(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w);
    ColorChannels prepareHSLW(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c w);
    ColorChannels prepareAuto(const String& colorspace, const ColorChannels& color);

    ColorChannels getRGBW();
    ColorChannels getHSVW();
    ColorChannels getHSLW();
    ColorChannels getAuto(const String& colorspace);

    String dumpFavoriteColor(const bool useWhite);
    ColorChannels decodeFavoriteColor(const String& formattedColor, bool* useWhitePtr);
    ColorChannels favoriteColorPreview(const String& colorspace, const String& formattedColor);
    void applyFavoriteColor(const String& formattedColor);

}
