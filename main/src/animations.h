#pragma once

#include "common_types.h"
#include "lib/ArduinoJson/ArduinoJson.h"

namespace animations {

    void checkTimer();
    bool startAnimation(unsigned id);
    void stopAnimation();
    void startAnimationFromJson(const JsonVariantConst animationSequence);

}
