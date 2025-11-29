#include <Arduino.h>
#include "timer_shutdown.h"
#include "inputs.h"
#include "common_types.h"
#include "outputs.h"

namespace timer_shutdown {

    std::uint64_t shutdown_time = 0;
    std::uint64_t timeout_value = 21600000;
    std::uint64_t fade_out_time = 10000;
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
        outputs::setFadeoutScalling(1);
    }


    void checkTimer() {
        if (!timer_shutdown_enabled) return;

        std::int64_t ctime = millis();

        if (ctime > shutdown_time) {
            if (!fading_out) {
                inputs::source_control = inputs::scFadingOut;
                fading_out = true;
            }
            std::int64_t diff = ctime - shutdown_time;
            if (diff < fade_out_time) {
                fixed64 fade_part = fixed64::fraction(fade_out_time - diff, fade_out_time);                
                outputs::setFadeoutScalling(fade_part * fade_part);
                outputs::writeOutput();
            } else if (diff < fade_out_time + 1000) {
                outputs::setFadeoutScalling(0);
                outputs::writeOutput();
            }
        } else {
            fading_out = false;
            outputs::setFadeoutScalling(1);
        } 
    }

}

