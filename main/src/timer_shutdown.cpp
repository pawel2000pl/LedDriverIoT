#include <Arduino.h>
#include "timer_shutdown.h"
#include "knobs.h"
#include "common_types.h"
#include "outputs.h"

namespace timer_shutdown {

    std::uint64_t shutdown_time = 0;
    std::uint64_t timeout_value = 21600000;
    std::uint64_t fade_out_time = 10000;
    ColorChannels fading_out_initial_colors = {0, 0, 0, 0};
    bool fading_out = false;
    bool timer_shutdown_enabled = false;


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto auto_shutdown = configuration["channels"]["autoShutdown"];
        timer_shutdown_enabled = auto_shutdown["enabled"].as<bool>();
        timeout_value = (std::uint64_t)(auto_shutdown["timeout"].as<int>()) * 1000;
        fade_out_time = (std::uint64_t)(auto_shutdown["fadeOutTime"].as<int>()) * 1000;
        resetTimer();
    }


    void resetTimer() {
        shutdown_time = millis() + timeout_value;
        fading_out = false;
    }


    void checkTimer() {
        if (!timer_shutdown_enabled) return;

        std::int64_t ctime = millis();

        if (ctime > shutdown_time) {
            if (!fading_out) {
                knobs::turnOff();
                fading_out_initial_colors = outputs::getColor();
                fading_out = true;
            }
            std::int64_t diff = ctime - shutdown_time;
            if (diff < fade_out_time) {
                fixed64 fade_part = fixed64::fraction(fade_out_time - diff, fade_out_time);
                fixed32_c fraction = fade_part * fade_part;
                ColorChannels color;
                color[0] = fading_out_initial_colors[0];
                color[1] = fading_out_initial_colors[1];
                color[2] = fading_out_initial_colors[2] * fraction;
                color[3] = fading_out_initial_colors[3] * fraction;
                outputs::setColor(color);
                outputs::writeOutput();
            } else if (diff < fade_out_time + 1000) {
                ColorChannels color;
                color[0] = fading_out_initial_colors[0];
                color[1] = fading_out_initial_colors[1];
                color[2] = 0;
                color[3] = 0;
                outputs::setColor(color);
                outputs::writeOutput();
            }
        } else fading_out = false;
    }

}

