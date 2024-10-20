#include "outputs.h"
#include <Arduino.h>

#include "ledc_driver.h"
#include "filter_functions.h"
#include "conversions.h"
#include "hardware_configuration.h"

std::vector<float> toFloatVector(const JsonVariantConst& source) {
    std::vector<float> result;
    unsigned size = source.size();
    result.reserve(size);
    for (unsigned i=0;i<size;i++)
        result.push_back(source[i].as<float>());
    return result;
}


namespace outputs {

    float hue = 0;
    float saturation = 0;
    float value = 0;
    float white = 0;

    bool invertOutputs = 0;
    int phaseMode = 0;
    float gateLoadingTime = 0;
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
        gateLoadingTime = hardwareConfiguration["gateLoadingTime"].as<float>();
        invertOutputs = hardwareConfiguration["invertOutputs"].as<bool>();
        unsigned outputFrequency = hardwareConfiguration["frequency"].as<unsigned>();
        checkNewFrequency(outputFrequency);

        const auto& scallingJson = configuration["hardware"]["scalling"];
        scalling[0] = scallingJson["red"].as<float>();
        scalling[1] = scallingJson["green"].as<float>();
        scalling[2] = scallingJson["blue"].as<float>();
        scalling[3] = scallingJson["white"].as<float>();

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
        float k = 1e-4;
        switch (phaseMode) {
            case 1: 
                for (int i=0;i<4;i++)
                    phases[i] = (1.f - values[i]) / 2;
                break;
            case 3: 
                k += values[3];
            case 2:
                for (int i=0;i<3;i++)
                    k += values[i];
                k = 1.f / k;
                float sum = 0;
                for (int i=0;i<4;i++) 
                    if (values[i] != 0) {
                        phases[i] = sum;
                        sum += fmod(sum + values[i] * k, 1.f);
                    }
                break;
        }
        return phases;
    }


    ColorChannels switchToTransistors(const ColorChannels& values) {
        ColorChannels results;
        for (int i=0;i<4;i++) {
            unsigned tc = transistorConnections[i];
            results[i] = tc < 4 ? values[tc] : 0;
        }
        return results;
    }


    void write2812(float r, float g, float b) {
        if (hardware::detectedHardware.ws2812 >= 0)
            neopixelWrite(hardware::detectedHardware.ws2812,round(255*g),round(255*r),round(255*b));
    }


    void writeOutput() {
        float r, g, b;
        hsvToRgb(hue, saturation, value, r, g, b);        
        ColorChannels filteredValues = {
            filters::outputRed(filters::globalOutput(r)) * scalling[0],
            filters::outputGreen(filters::globalOutput(g)) * scalling[1],
            filters::outputBlue(filters::globalOutput(b)) * scalling[2],
            filters::outputWhite(filters::globalOutput(white)) * scalling[3]
        };
        write2812(filteredValues[0], filteredValues[1], filteredValues[2]);
        ColorChannels periods = switchToTransistors(getPeriods(filteredValues));
        ColorChannels phases = switchToTransistors(getPhases(filteredValues));
        for (int i=0;i<4;i++)
            setLedC(
                hardware::detectedHardware.outputs[i], 
                i, 
                periods[i], 
                phases[i],
                invertOutputs
            );
    }


    void setColor(float h, float s, float v, float w) {
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

}
