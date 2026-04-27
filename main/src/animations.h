#pragma once

#include "common_types.h"
#include "lib/ArduinoJson/ArduinoJson.h"

namespace animations {

    void checkTimer();
    bool startAnimation(unsigned id, bool checkMinimum=true);
    void startAnimationFromJson(const JsonVariantConst animationSequence, bool checkMinimum=true);
    void stopAnimation();

}
