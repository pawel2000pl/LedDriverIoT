#include "outputs.h"
#include <Arduino.h>

#include "constrain.h"
#include "conversions.h"
#include "ledc_driver.h"
#include "filter_functions.h"
#include "hardware_configuration.h"

std::vector<fraction32> toFloatVector(const JsonVariantConst& source) {
    std::vector<fraction32> result;
    unsigned size = source.size();
    result.reserve(size);
    for (unsigned i=0;i<size;i++)
        result.push_back(source[i].as<fraction32>());
    return result;
}


namespace outputs {

    fraction32 hue = 0;
    fraction32 saturation = 0;
    fraction32 value = 0;
    fraction32 white = 0;

    bool invertOutputs = 0;
    int phaseMode = 0;
    fraction32 gateLoadingTime = 0;
    ColorChannels scalling = {1.0f, 1.0f, 1.0f, 1.0f};
    std::array<unsigned, 4> transistorConnections;

    namespace filters {
        FloatFunction outputRed;
        FloatFunction outputGreen;
        FloatFunction outputBlue;
        FloatFunction outputWhite;
        FloatFunction globalOutput;
    }

    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto& filters = configuration["filters"];
        const auto& outputFilters = filters["outputFilters"];

        filters::outputRed = mixFilterFunctions(toFloatVector(outputFilters["red"]));
        filters::outputGreen = mixFilterFunctions(toFloatVector(outputFilters["green"]));
        filters::outputBlue = mixFilterFunctions(toFloatVector(outputFilters["blue"]));
        filters::outputWhite = mixFilterFunctions(toFloatVector(outputFilters["white"]));   
        filters::globalOutput = mixFilterFunctions(toFloatVector(filters["globalOutputFilters"])); 

        const auto& hardwareConfiguration = configuration["hardware"];
        phaseMode = hardwareConfiguration["phaseMode"].as<int>();
        gateLoadingTime = hardwareConfiguration["gateLoadingTime"].as<fraction32>();
        invertOutputs = hardwareConfiguration["invertOutputs"].as<bool>();
        unsigned outputFrequency = hardwareConfiguration["frequency"].as<unsigned>();
        checkNewFrequency(outputFrequency);

        const auto& scallingJson = configuration["hardware"]["scalling"];
        scalling[0] = scallingJson["red"].as<fraction32>();
        scalling[1] = scallingJson["green"].as<fraction32>();
        scalling[2] = scallingJson["blue"].as<fraction32>();
        scalling[3] = scallingJson["white"].as<fraction32>();

        const auto& transistorConfiguration = configuration["hardware"]["transistorConfiguration"];
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



    ColorChannels getPeriods(const ColorChannels& values) {
        ColorChannels periods;
        for (int i=0;i<4;i++)
            periods[i] = addGateLoadingTime(values[i], gateLoadingTime);
        return periods;
    }
    

    ColorChannels getPhases(const ColorChannels& values) {
        ColorChannels phases = {0, 0, 0, 0};
        fraction32 k = 0;
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
                fraction32 sum = 0;
                for (int i=0;i<4;i++) 
                    if (values[i] != 0) {
                        sum = sum % 1;
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
            results[i] = tc < 4 ? values[tc] : (fraction32)0;
        }
        return results;
    }


    void writeOutput() {
        fraction32 r, g, b;
        hsvToRgb(hue, saturation, value, r, g, b);        
        ColorChannels filteredValues = {
            filters::outputRed(filters::globalOutput(r)) * scalling[0],
            filters::outputGreen(filters::globalOutput(g)) * scalling[1],
            filters::outputBlue(filters::globalOutput(b)) * scalling[2],
            filters::outputWhite(filters::globalOutput(white)) * scalling[3]
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


    void setColor(fraction32 h, fraction32 s, fraction32 v, fraction32 w) {
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
        fraction32 r, g, b;
        hsvToRgb(hue, saturation, value, r, g, b); 
        fraction32 scale0 = constrain<fraction32>(filters::outputRed(filters::globalOutput(r)) * scalling[0] / max<fraction32>(filters::outputRed(filters::globalOutput(1)), std::numeric_limits<fraction32>::min()), 0, 1);
        fraction32 scale1 = constrain<fraction32>(filters::outputGreen(filters::globalOutput(g)) * scalling[1] / max<fraction32>(filters::outputGreen(filters::globalOutput(1)), std::numeric_limits<fraction32>::min()), 0, 1);
        fraction32 scale2 = constrain<fraction32>(filters::outputBlue(filters::globalOutput(b)) * scalling[2] / max<fraction32>(filters::outputBlue(filters::globalOutput(1)), std::numeric_limits<fraction32>::min()), 0, 1);
        fraction32 scale3 = constrain<fraction32>(filters::outputWhite(filters::globalOutput(white)) * scalling[3] / max<fraction32>(filters::outputWhite(filters::globalOutput(1)), std::numeric_limits<fraction32>::min()), 0, 1);
        return {scale0, scale1, scale2, scale3};
    }

}
