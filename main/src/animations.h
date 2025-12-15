#pragma once

#include "lib/ArduinoJson/ArduinoJson.h"

namespace animations {

    void checkTimer();
    void startAnimation(unsigned id);
    void startAnimationFromJson(const JsonVariant& animationSequence);

}
