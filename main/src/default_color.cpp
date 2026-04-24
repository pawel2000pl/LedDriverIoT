#include "default_color.h"
#include "common_types.h"
#include "inputs.h"
#include "outputs.h"
#include "timer_shutdown.h"
#include "animations.h"
#include "knobs.h"
#include "json_utils.h"


namespace default_color {

    bool enableDefaultColor = false;
    ColorChannels defaultColor = {0,0,0,0};
    int defaultAnimation = -1;


    void updateConfiguration(const JsonVariantConst configuration) {
        const auto channelsJson = configuration["channels"];
        enableDefaultColor = channelsJson["defaultColorEnabled"].as<bool>();
        const auto defaultColorJson = channelsJson["defaultColor"];
        defaultColor = {
            defaultColorJson["hue"].as<fixed32_c>(),
            defaultColorJson["saturation"].as<fixed32_c>(),
            defaultColorJson["value"].as<fixed32_c>(),
            defaultColorJson["white"].as<fixed32_c>()
        };
        defaultAnimation = channelsJson["defaultAnimation"].as<int>();
    }


    void setDefaultColor() {
        if (!enableDefaultColor) return;
        inputs::source_control = inputs::scWeb;
        inputs::setHSVW(defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]);
        if (defaultAnimation >= 0 && animations::startAnimation(defaultAnimation))
            return;
        timer_shutdown::resetTimer();
    }


}