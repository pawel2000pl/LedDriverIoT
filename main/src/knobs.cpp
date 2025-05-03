#include "knobs.h"
#include <functional>
#include <array>
#include "hardware_configuration.h"
#include "inputs.h"
#include "constrain.h"
#include "outputs.h"
#include "hardware_configuration.h"
#include "common_types.h"

namespace knobs {

    bool knobMode = true;
    ColorChannels lastKnobValues = {0,0,0,0};
    String knobColorspace = "hsv";
    std::function<fixed64(fixed64)> applyBias = [](fixed64 x){return x;};
    std::array<unsigned, 4> potentionemterMapping = {0, 0, 0, 0};

    fixed64 epsilon = 0;
    fixed64 reduction = 0.1;
    ColorChannels knobsAmortisation = {0,0,0,0};
    
    const fixed64 analogResolution = 1.44f / ANALOG_READ_MAX;
    const char* colorspaces[] = {"hsv", "hsl", "rgb"};
    const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};

    bool enableDefaultColor = false;
    ColorChannels defaultColor = {0,0,0,0};

    bool settingsInLock = false;
        
    hw_timer_t * timer = NULL;

    void turnOff() {
        knobMode = false;
    }


    void setLock(bool lockState) {
        settingsInLock = lockState;
    }


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto bias = configuration["hardware"]["bias"];
        const auto channelsJson = configuration["channels"];
        fixed64 biasUp = bias["up"].as<fixed64>();
        fixed64 biasDown = bias["down"].as<fixed64>();
        epsilon = configuration["hardware"]["knobActivateDelta"].as<fixed64>();
        reduction = exp(-abs(configuration["hardware"]["knobsNoisesReduction"].as<fixed64>()));
        applyBias = [=](fixed64 x) { return constrain<fixed64>((x - biasDown) / (1 - biasUp - biasDown), 0, 1); };
        knobColorspace = channelsJson["knobMode"].as<String>();
        const char** channelsInCurrentColorspace = channels[0];
        for (int i=0;i<3;i++)
            if (knobColorspace == colorspaces[i])
                channelsInCurrentColorspace = channels[i];
        const auto& potentionemterConfiguration = configuration["hardware"]["potentionemterConfiguration"];
        for (int i=0;i<4;i++)
            potentionemterMapping[i] = (unsigned)potentionemterConfiguration[channelsInCurrentColorspace[i]].as<unsigned>();

        enableDefaultColor = channelsJson["defaultColorEnabled"].as<bool>();
        const auto defaultColorJson = channelsJson["defaultColor"];
        defaultColor = {
            defaultColorJson["hue"].as<fixed64>(),
            defaultColorJson["saturation"].as<fixed64>(),
            defaultColorJson["value"].as<fixed64>(),
            defaultColorJson["white"].as<fixed64>()
        };
    }


    void setDefaultColor() {
        if (!enableDefaultColor) return;
        delay(100); // let timer do some iterations
        knobMode = false;
        inputs::setHSVW(defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]);
        outputs::writeOutput();
    }


    void setFromKnobs(const ColorChannels& values) {
        const fixed64 fixedValues[6] = {applyBias(values[0]), applyBias(values[1]), applyBias(values[2]), applyBias(values[3]), 0, 1};
        ColorChannels outputChannels;
        for (int i=0;i<4;i++)
            outputChannels[i] = potentionemterMapping[i] < 6 ? fixedValues[potentionemterMapping[i]] : (fixed64)0;
        inputs::setAuto(knobColorspace, outputChannels);
        outputs::writeOutput();
    }


    fixed64 maxAbsDifference(const ColorChannels& a, const ColorChannels& b) {
        fixed64 result = 0;
        for (int i=0;i<4;i++) {
            fixed64 test = std::abs(a[i] - b[i]);
            if (test > result)
                result = test;
        }
        return result;
    }


    void checkIfKnobsMoved(const ColorChannels& values, bool force) {
        fixed64 md = maxAbsDifference(values, lastKnobValues);
        if (md > epsilon || (knobMode && md > analogResolution) || force) {
            knobMode = true;
            for (int i=0;i<4;i++)
                lastKnobValues[i] = values[i];
            setFromKnobs(values);
        }
    }


    void check(bool force) {
        fixed64 cReduction = force ? (fixed64)1 : reduction;
        fixed64 opReductionc = 1 - cReduction;
        for (int i=0;i<4;i++)
            knobsAmortisation[i] = cReduction * (fixed64)(hardware_configuration.potentiometers[i].read()) + opReductionc * (fixed64)knobsAmortisation[i];
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
