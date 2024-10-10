#include "knobs.h"
#include <functional>
#include <array>
#include "hardware_configuration.h"
#include "light_pipeline.h"
#include "constrain.h"

namespace knobs {

    bool knobMode = true;
    ColorChannels lastKnobValues = {0,0,0,0};
    String knobColorspace = "hsv";
    std::function<float(float)> applyBias = [](float x){return x;};
    std::array<unsigned, 4> potentionemterMapping = {0, 0, 0, 0};

    float epsilon = 0;
    float reduction = 0.1;
    ColorChannels knobsAmortisation = {0,0,0,0};
    
    const float analogResolution = 1.44f / (float)ANALOG_READ_MAX;
    const char* colorspaces[] = {"hsv", "hsl", "rgb"};
    const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};
        

    void turnOff() {
        knobMode = false;
    }


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto& bias = configuration["hardware"]["bias"];
        float biasUp = bias["up"].as<float>();
        float biasDown = bias["down"].as<float>();
        epsilon = configuration["hardware"]["knobActivateDelta"].as<float>();
        reduction = exp(-abs(configuration["hardware"]["knobsNoisesReduction"].as<float>()));
        applyBias = [=](float x) { return constrain<float>((x - biasDown) / (1.f - biasUp - biasDown), 0, 1); };
        knobColorspace = configuration["channels"]["knobMode"].as<String>();
        const char** channelsInCurrentColorspace = channels[0];
        for (int i=0;i<3;i++)
            if (knobColorspace == colorspaces[i])
                channelsInCurrentColorspace = channels[i];
        const auto& potentionemterConfiguration = configuration["hardware"]["potentionemterConfiguration"];
        for (int i=0;i<4;i++)
            potentionemterMapping[i] = (unsigned)potentionemterConfiguration[channelsInCurrentColorspace[i]].as<unsigned>();
    }


    void setFromKnobs(const ColorChannels& values) {
        const float fixedValues[6] = {applyBias(values[0]), applyBias(values[1]), applyBias(values[2]), applyBias(values[3]), 0, 1};
        ColorChannels outputChannels;
        for (int i=0;i<4;i++)
            outputChannels[i] = potentionemterMapping[i] < 6 ? fixedValues[potentionemterMapping[i]] : 0;
        pipeline::setAuto(knobColorspace, outputChannels);
        pipeline::writeOutput();
    }


    float maxAbsDifference(const ColorChannels& a, const ColorChannels& b) {
        float result = 0;
        for (int i=0;i<4;i++) {
            float test = abs(a[i] - b[i]);
            if (test > result)
                result = test;
        }
        return result;
    }


    void checkIfKnobsMoved(const ColorChannels& values) {
        float md = maxAbsDifference(values, lastKnobValues);
        if (md > epsilon || (knobMode && md > analogResolution)) {
            knobMode = true;
            for (int i=0;i<4;i++)
                lastKnobValues[i] = values[i];
            setFromKnobs(values);
        }
    }


    void check() {
        ColorChannels values;
        for (int i=0;i<4;i++)
            knobsAmortisation[i] = reduction * POTENTIOMETER_HARDWARE_ACTIONS[i].read() + (1.f-reduction) * knobsAmortisation[i];
        checkIfKnobsMoved(knobsAmortisation);
    }

}