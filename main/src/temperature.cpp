#include "temperature.h"

#include <Arduino.h>
#include <driver/temperature_sensor.h>

#include "hardware_configuration.h"

namespace temperature {

    bool fanStatus = false;
    uint8_t measureTemperature = 0;
    temperature_sensor_handle_t temp_handle = NULL;


    float TemperatureResults::max() {
        float v = internal;
        for (auto x : external) 
            if (x > v)
                v = x;
        return v;
    }


    bool TemperatureResults::tooHot() {
        return internal > RESTART_TEMP;
    }


    void init() {
        temperature_sensor_config_t temp_sensor = {
                .range_min = 20,
                .range_max = 100,
                .clk_src   = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT,
                .flags     = { .allow_pd = false }
        };
        temperature_sensor_install(&temp_sensor, &temp_handle);
    }


    float readTemperature(float x) {
        float RT = x * THERMISTOR_IN_SERIES_RESISTOR / (1.f - x);
        float Tk = 1.0/(log(RT/THERMISTOR_R0)/4050.0 + 1.0/THERMISTOR_T0);   
        return Tk - 273.15;
    }


    TemperatureResults readTemperatures() {
        TemperatureResults result;

        temperature_sensor_enable(temp_handle);
        temperature_sensor_get_celsius(temp_handle, &result.internal);
        temperature_sensor_disable(temp_handle);

        for (unsigned i=0;i<4;i++) {
            const auto& actions = hardware_configuration.thermistors[i];
            if (!actions.enabled) continue;
            result.external[i] = readTemperature(actions.read()/2);
        }

        return result;
    }


    TemperatureResults check() {
        TemperatureResults temps = readTemperatures();
        float temp_max = temps.max();
        fanStatus = ((fanStatus) && (temp_max > FAN_TURN_OFF_TEMP)) || ((!fanStatus) && (temp_max > FAN_TURN_ON_TEMP));
        digitalWrite(hardware_configuration.fanPin, fanStatus ? HIGH : LOW);
        return temps;
    }


    void block_until_is_ok() {
        TemperatureResults temps;
        while (true) {
            temps = check();
            if (temps.internal <= ALLOW_ON_MAX_TEMP && temps.internal >= ALLOW_ON_MIN_TEMP)
                break;
            delay(100);
        }
    }

}
