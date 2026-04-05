#pragma once

#include <list>
#include <string>
#include <Arduino.h>
#include <functional>
#include "lib/ArduinoJson/ArduinoJson.h"

namespace modules {

    extern std::list<std::function<void()>> taskQueue;
    extern String webColorSpace;
    extern bool colorKnobEnabled;
    extern bool whiteKnobEnabled;

    void updateModules(JsonVariant configuration);
    void updateModules();
    void execTaskQueue();

}

