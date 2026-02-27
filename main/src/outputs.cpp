#include "outputs.h"
#include <Arduino.h>

#include "constrain.h"
#include "conversions.h"
#include "ledc_driver.h"
#include "filter_functions.h"
#include "hardware_configuration.h"

std::vector<fixed32_f> toFixedpointVector(const JsonVariantConst source) {
    std::vector<fixed32_f> result;
    unsigned size = source.size();
    result.reserve(size);
    for (unsigned i=0;i<size;i++)
        result.push_back(source[i].as<fixed32_f>());
    return result;
}


namespace outputs {

    fixed32_c hue = 0;
    fixed32_c saturation = 0;
    fixed32_c value = 0;
    fixed32_c white = 0;

    bool invertOutputs = 0;
    int phaseMode = 0;
    fixed32_c gateLoadingTime = 0;
    ColorChannels scalling = {1.0f, 1.0f, 1.0f, 1.0f};
    fixed32_c fadeoutScalling = 1;
    std::array<unsigned, 4> transistorConnections;

    namespace filters {
        MixedFunction outputRed;
        MixedFunction outputGreen;
        MixedFunction outputBlue;
        MixedFunction outputWhite;
        MixedFunction globalOutput;

    }

    void updateConfiguration(const JsonVariantConst configuration) {
        const auto filters = configuration["filters"];
        const auto outputFilters = filters["outputFilters"];

        filters::outputRed = mixFilterFunctions(toFixedpointVector(outputFilters["red"]));
        filters::outputGreen = mixFilterFunctions(toFixedpointVector(outputFilters["green"]));
        filters::outputBlue = mixFilterFunctions(toFixedpointVector(outputFilters["blue"]));
        filters::outputWhite = mixFilterFunctions(toFixedpointVector(outputFilters["white"]));   
        filters::globalOutput = mixFilterFunctions(toFixedpointVector(filters["globalOutputFilters"])); 

        const auto hardwareConfiguration = configuration["hardware"];
        phaseMode = hardwareConfiguration["phaseMode"].as<int>();
        gateLoadingTime = hardwareConfiguration["gateLoadingTime"].as<fixed32_c>();
        invertOutputs = hardwareConfiguration["invertOutputs"].as<bool>();
        unsigned outputFrequency = hardwareConfiguration["frequency"].as<unsigned>();
        checkNewFrequency(outputFrequency);

        const auto scallingJson = configuration["hardware"]["scalling"];
        scalling[0] = scallingJson["red"].as<fixed32_c>();
        scalling[1] = scallingJson["green"].as<fixed32_c>();
        scalling[2] = scallingJson["blue"].as<fixed32_c>();
        scalling[3] = scallingJson["white"].as<fixed32_c>();

        const auto transistorConfiguration = configuration["hardware"]["transistorConfiguration"];
        char key[] = {'o', 'u', 't', 'p', 'u', 't', ' ', '#', 0};
        int idx;
        for (idx=0;key[idx]!='#';idx++);
        for (int i=0;i<4;i++) {
            key[idx] = '0' + i;
            unsigned selectedChannel = (unsigned)transistorConfiguration[(const char*)key].as<int>();
            transistorConnections[i] = selectedChannel <= 4 ? selectedChannel : 5;
        }

        writeOutput();
    }


    void setFadeoutScalling(fixed32_c value) {
        fadeoutScalling = value;
    }


    ColorChannels getPeriods(const ColorChannels& values) {
        ColorChannels periods;
        for (int i=0;i<4;i++)
            periods[i] = addGateLoadingTime(values[i], gateLoadingTime);
        return periods;
    }
    

    ColorChannels getPhases(const ColorChannels& values) {
        ColorChannels phases = {0, 0, 0, 0};
        fixed32_c k = 0;
        switch (phaseMode) {
            case 1: 
                for (int i=0;i<4;i++)
                    phases[i] = (1 - values[i]) / 2;
                break;
            case 3: 
                k += values[3];
                // fall through
            case 2:
                for (int i=0;i<3;i++)
                    k += values[i];
                if (k == 0) break;
                k = 1 / k;
                fixed32_c sum = 0;
                for (int i=0;i<4;i++) 
                    if (values[i] != 0) {
                        sum %= 1;
                        phases[i] = sum;
                        sum += values[i] * k;
                    }
                break;
        }
        return phases;
    }


    ColorChannels switchToTransistors(const ColorChannels& values) {
        ColorChannels results;
        for (int i=0;i<4;i++) {
            unsigned tc = transistorConnections[i];
            results[i] = tc < 4 ? values[tc] : (fixed32_c)0;
        }
        return results;
    }


    void writeOutput() {
        fixed32_c r, g, b;
        hsvToRgb(hue, saturation, value * fadeoutScalling, r, g, b);
        ColorChannels filteredValues = {
            (fixed32_c)filters::outputRed(filters::globalOutput(r)) * scalling[0],
            (fixed32_c)filters::outputGreen(filters::globalOutput(g)) * scalling[1],
            (fixed32_c)filters::outputBlue(filters::globalOutput(b)) * scalling[2],
            (fixed32_c)filters::outputWhite(filters::globalOutput(white)) * scalling[3] * fadeoutScalling
        };
        ColorChannels periods = switchToTransistors(getPeriods(filteredValues));
        ColorChannels phases = switchToTransistors(getPhases(filteredValues));
        for (int i=0;i<4;i++)
            setLedC(
                hardware_configuration.outputs[i], 
                i, 
                periods[i], 
                phases[i],
                invertOutputs
            );
    }


    void setColor(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w) {
        hue = h;
        saturation = s;
        value = v;
        white = w;
    }


    void setColor(const ColorChannels& channels) {
        hue = channels[0];
        saturation = channels[1];
        value = channels[2];
        white = channels[3];
    }


    ColorChannels getColor() {
        return {hue, saturation, value, white};
    }


    ColorChannels getTailoredScalling() {
        fixed32_c r, g, b;
        hsvToRgb(hue, saturation, value, r, g, b); 
        fixed32_c scale0 = constrain<fixed32_c>((fixed32_c)filters::outputRed(filters::globalOutput(r)) * scalling[0] / max<fixed32_c>(filters::outputRed(filters::globalOutput(1)), std::numeric_limits<fixed32_c>::min()), 0, 1);
        fixed32_c scale1 = constrain<fixed32_c>((fixed32_c)filters::outputGreen(filters::globalOutput(g)) * scalling[1] / max<fixed32_c>(filters::outputGreen(filters::globalOutput(1)), std::numeric_limits<fixed32_c>::min()), 0, 1);
        fixed32_c scale2 = constrain<fixed32_c>((fixed32_c)filters::outputBlue(filters::globalOutput(b)) * scalling[2] / max<fixed32_c>(filters::outputBlue(filters::globalOutput(1)), std::numeric_limits<fixed32_c>::min()), 0, 1);
        fixed32_c scale3 = constrain<fixed32_c>((fixed32_c)filters::outputWhite(filters::globalOutput(white)) * scalling[3] / max<fixed32_c>(filters::outputWhite(filters::globalOutput(1)), std::numeric_limits<fixed32_c>::min()), 0, 1);
        return {scale0, scale1, scale2, scale3};
    }

}
