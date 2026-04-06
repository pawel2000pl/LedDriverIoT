#include "knobs.h"
#include <functional>
#include <array>
#include "hardware_configuration.h"
#include "inputs.h"
#include "constrain.h"
#include "outputs.h"
#include "hardware_configuration.h"
#include "common_types.h"
#include "timer_shutdown.h"

namespace knobs {

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



    void updateConfiguration(const JsonVariantConst configuration) {
        const auto bias = configuration["hardware"]["bias"];
        const auto channelsJson = configuration["channels"];
        fixed32_c biasUp = bias["up"].as<fixed32_c>();
        fixed32_c biasDown = bias["down"].as<fixed32_c>();
        epsilon = configuration["hardware"]["knobActivateDelta"].as<fixed32_c>();
        reduction = exp(-std::abs(configuration["hardware"]["knobsNoisesReduction"].as<float>()));
        applyBias = [=](fixed32_c x) { return constrain<fixed32_c>((x - biasDown) / (1 - biasUp - biasDown), 0, 1); };
        knobColorspace = channelsJson["knobMode"].as<String>();
        const char** channelsInCurrentColorspace = channels[0];
        for (int i=0;i<3;i++)
            if (knobColorspace == colorspaces[i])
                channelsInCurrentColorspace = channels[i];
        const auto potentionemterConfiguration = configuration["hardware"]["potentionemterConfiguration"];
        for (int i=0;i<4;i++)
            potentionemterMapping[i] = (unsigned)potentionemterConfiguration[channelsInCurrentColorspace[i]].as<unsigned>();
    }


    void setFromKnobs(const ColorChannels& values) {
        const fixed32_c fixedValues[6] = {applyBias(values[0]), applyBias(values[1]), applyBias(values[2]), applyBias(values[3]), 0, 1};
        ColorChannels outputChannels;
        for (int i=0;i<4;i++)
            outputChannels[i] = potentionemterMapping[i] < 6 ? fixedValues[potentionemterMapping[i]] : (fixed32_c)0;        
        inputs::setAuto(knobColorspace, outputChannels);
        outputs::writeOutput();
    }


    template<typename T, typename U=T>
    T maxAbsDifference(const std::array<T, 4>& a, const std::array<U, 4>& b) {
        T result = 0;
        for (int i=0;i<4;i++) {
            T test = std::abs(a[i] - b[i]);
            if (test > result)
                result = test;
        }
        return result;
    }

    
    template<typename T>
    T sqr(T x) {
        return x * x;
    }


    void checkIfKnobsMoved(const ColorChannels& values, bool force) {
        fixed32_c md = maxAbsDifference<fixed32_c>(values, lastKnobValues);
        bool largeChange = md > epsilon;
        if (largeChange || force)
            timer_shutdown::resetTimer();
        if (largeChange || (inputs::source_control == inputs::scKnobs && md > analogResolution) || force) {
            inputs::source_control = inputs::scKnobs;
            for (int i=0;i<4;i++)
                lastKnobValues[i] = values[i];
            setFromKnobs(values);
        }
    }


    void check(bool force) {
        std::array<fixed64, 4> readed;
        for (int i=0;i<4;i++)
            readed[i] = hardware::configuration->potentiometers[i].read();
        fixed64 md = maxAbsDifference<fixed64, fixed32_c>(readed, knobsAmortisation);
        fixed64 cReduction = force ? (fixed32_c)1 : reduction;
        fixed64 opReduction = 1 - cReduction;
        opReduction *= sqr<fixed64>(1-md);
        cReduction = 1 - opReduction;
        for (int i=0;i<4;i++)
            knobsAmortisation[i] = cReduction * readed[i] + opReduction * knobsAmortisation[i];
        checkIfKnobsMoved(knobsAmortisation, force);
    }


    void init() {
        check(true);
        for (int i=0;i<20;i++) {
            delay(5);
            check(false);
        }
    }


    void checkTimer() {
        check(false);
    }


}
