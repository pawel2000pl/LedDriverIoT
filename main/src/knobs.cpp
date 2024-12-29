#include "knobs.h"
#include <functional>
#include <array>
#include "hardware_configuration.h"
#include "inputs.h"
#include "constrain.h"
#include "outputs.h"
#include "hardware_configuration.h"

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

    bool settingsInLock = false;
        
    hw_timer_t * timer = NULL;

    void turnOff() {
        knobMode = false;
    }


    void setLock(bool lockState) {
        settingsInLock = lockState;
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
        inputs::setAuto(knobColorspace, outputChannels);
        outputs::writeOutput();
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


    void checkIfKnobsMoved(const ColorChannels& values, bool force) {
        float md = maxAbsDifference(values, lastKnobValues);
        if (md > epsilon || (knobMode && md > analogResolution) || force) {
            knobMode = true;
            for (int i=0;i<4;i++)
                lastKnobValues[i] = values[i];
            setFromKnobs(values);
        }
    }


    void check(bool force) {
        float cReduction = force ? 1.f : reduction;
        float opReductionc = 1.f - cReduction;
        for (int i=0;i<4;i++)
            knobsAmortisation[i] = cReduction * hardware_configuration.potentiometers[i].read() + opReductionc * knobsAmortisation[i];
        checkIfKnobsMoved(knobsAmortisation, force);
    }


    void ARDUINO_ISR_ATTR timerCheck() {
        if (settingsInLock) return;
        check(false);
    }


    void attachTimer() {
        if (timer) return;
        timer = timerBegin(1000000);
        timerAttachInterrupt(timer, &timerCheck);
        timerAlarm(timer, 16667, true, 0);
    }

}
