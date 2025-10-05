#include "knobs.h"
#include <functional>
#include <array>
#include "hardware_configuration.h"
#include "inputs.h"
#include "constrain.h"
#include "outputs.h"
#include "hardware_configuration.h"
#include "common_types.h"
#include "taylormath.h"
#include "timer_shutdown.h"

namespace knobs {

    bool knobMode = true;
    ColorChannels lastKnobValues = {0,0,0,0};
    String knobColorspace = "hsv";
    std::function<fixed32_c(fixed32_c)> applyBias = [](fixed32_c x){return x;};
    std::array<unsigned, 4> potentionemterMapping = {0, 0, 0, 0};

    fixed32_c epsilon = 0;
    fixed32_c reduction = 0.1;
    ColorChannels knobsAmortisation = {0,0,0,0};
    
    const fixed32_c analogResolution = 1.44f / ANALOG_READ_MAX;
    const char* colorspaces[] = {"hsv", "hsl", "rgb"};
    const char* channels[][4] = {{"hue", "saturation", "value", "white"}, {"hue", "saturation", "lightness", "white"}, {"red", "green", "blue", "white"}};

    bool enableDefaultColor = false;
    ColorChannels defaultColor = {0,0,0,0};


    void turnOff() {
        knobMode = false;
    }


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto bias = configuration["hardware"]["bias"];
        const auto channelsJson = configuration["channels"];
        fixed32_c biasUp = bias["up"].as<fixed32_c>();
        fixed32_c biasDown = bias["down"].as<fixed32_c>();
        epsilon = configuration["hardware"]["knobActivateDelta"].as<fixed32_c>();
        reduction = taylor::exp(-std::abs(configuration["hardware"]["knobsNoisesReduction"].as<fixed32_c>()));
        applyBias = [=](fixed32_c x) { return constrain<fixed32_c>((x - biasDown) / (1 - biasUp - biasDown), 0, 1); };
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
            defaultColorJson["hue"].as<fixed32_c>(),
            defaultColorJson["saturation"].as<fixed32_c>(),
            defaultColorJson["value"].as<fixed32_c>(),
            defaultColorJson["white"].as<fixed32_c>()
        };
    }


    void setDefaultColor() {
        if (!enableDefaultColor) return;
        delay(100); // let timer do some iterations
        knobMode = false;
        timer_shutdown::resetTimer();
        inputs::setHSVW(defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]);
        outputs::writeOutput();
    }


    void setFromKnobs(const ColorChannels& values) {
        const fixed32_c fixedValues[6] = {applyBias(values[0]), applyBias(values[1]), applyBias(values[2]), applyBias(values[3]), 0, 1};
        ColorChannels outputChannels;
        for (int i=0;i<4;i++)
            outputChannels[i] = potentionemterMapping[i] < 6 ? fixedValues[potentionemterMapping[i]] : (fixed32_c)0;        
        inputs::setAuto(knobColorspace, outputChannels);
        outputs::writeOutput();
    }


    fixed32_c maxAbsDifference(const ColorChannels& a, const ColorChannels& b) {
        fixed32_c result = 0;
        for (int i=0;i<4;i++) {
            fixed32_c test = std::abs(a[i] - b[i]);
            if (test > result)
                result = test;
        }
        return result;
    }


    void checkIfKnobsMoved(const ColorChannels& values, bool force) {
        fixed32_c md = maxAbsDifference(values, lastKnobValues);
        bool largeChange = md > epsilon;
        if (largeChange || force)
            timer_shutdown::resetTimer();
        if (largeChange || (knobMode && md > analogResolution) || force) {
            knobMode = true;
            for (int i=0;i<4;i++)
                lastKnobValues[i] = values[i];
            setFromKnobs(values);
        }
    }


    void check(bool force) {
        fixed32_c cReduction = force ? (fixed32_c)1 : reduction;
        fixed32_c opReductionc = 1 - cReduction;
        for (int i=0;i<4;i++)
            knobsAmortisation[i] = cReduction * (fixed32_c)(hardware_configuration.potentiometers[i].read()) + opReductionc * (fixed32_c)knobsAmortisation[i];
        checkIfKnobsMoved(knobsAmortisation, force);
    }


    void checkTimer() {
        check(false);
    }


}
