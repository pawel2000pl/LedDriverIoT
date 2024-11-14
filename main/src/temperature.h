#pragma once

#include <array>

#define FAN_TURN_ON_TEMP 70
#define FAN_TURN_OFF_TEMP 50
#define THERMISTOR_CONST 4050.0f
#define THERMISTOR_R0 47000
#define THERMISTOR_IN_SERIES_RESISTOR 47000
#define THERMISTOR_T0 (25.0f + 273.15f)


namespace temperature {

    struct TemperatureResults {
        float internal = 0;
        std::array<float, 4> external = {0, 0, 0, 0};
        float max();
    };

    void init();
    void check();
    TemperatureResults readTemperatures();

}
