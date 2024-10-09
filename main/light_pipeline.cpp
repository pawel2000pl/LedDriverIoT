#include <cmath>
#include "light_pipeline.h"
#include "conversions.h"
#include "ledc_driver.h"
#include "filter_functions.h"

namespace pipeline {

    const int LED_GPIO_OUTPUTS[] = {D7, D8, D9, D10};

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
        FloatFunction inputSaturation;
        FloatFunction inputHue;
        FloatFunction inputValue;
        FloatFunction inputLightness;
        FloatFunction inputRed;
        FloatFunction inputGreen;
        FloatFunction inputBlue;
        FloatFunction inputWhite;
        FloatFunction globalInput;

        FloatFunction outputRed;
        FloatFunction outputGreen;
        FloatFunction outputBlue;
        FloatFunction outputWhite;
        FloatFunction globalOutput;

        FloatFunction invertedInputHue;
        FloatFunction invertedInputSaturation;
        FloatFunction invertedInputValue;
        FloatFunction invertedInputLightness;
        FloatFunction invertedInputRed;
        FloatFunction invertedInputGreen;
        FloatFunction invertedInputBlue;
        FloatFunction invertedInputWhite;
        FloatFunction invertedGlobalInput;
    }


    std::vector<float> toFloatVector(const JsonVariantConst& source) {
        std::vector<float> result;
        unsigned size = source.size();
        result.reserve(size);
        for (unsigned i=0;i<size;i++)
            result.push_back(source[i].as<float>());
        return result;
    }


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto& filters = configuration["filters"];
        const auto& inputFilters = filters["inputFilters"];
        const auto& outputFilters = filters["outputFilters"];
                
        filters::inputSaturation = mixFilterFunctions(toFloatVector(inputFilters["saturation"]));
        filters::inputHue = periodizeFunction(mixFilterFunctions(toFloatVector(inputFilters["hue"])), 6);
        filters::inputValue = mixFilterFunctions(toFloatVector(inputFilters["value"]));
        filters::inputLightness = mixFilterFunctions(toFloatVector(inputFilters["lightness"]));
        filters::inputRed = mixFilterFunctions(toFloatVector(inputFilters["red"]));
        filters::inputGreen = mixFilterFunctions(toFloatVector(inputFilters["green"]));
        filters::inputBlue = mixFilterFunctions(toFloatVector(inputFilters["blue"]));
        filters::inputWhite = mixFilterFunctions(toFloatVector(inputFilters["white"]));
        filters::globalInput = mixFilterFunctions(toFloatVector(filters["globalInputFilters"]));  

        filters::outputRed = mixFilterFunctions(toFloatVector(outputFilters["red"]));
        filters::outputGreen = mixFilterFunctions(toFloatVector(outputFilters["green"]));
        filters::outputBlue = mixFilterFunctions(toFloatVector(outputFilters["blue"]));
        filters::outputWhite = mixFilterFunctions(toFloatVector(outputFilters["white"]));   
        filters::globalOutput = mixFilterFunctions(toFloatVector(filters["globalOutputFilters"])); 

        filters::invertedInputHue = createInverseFunction(filters::inputHue);
        filters::invertedInputSaturation = createInverseFunction(filters::inputSaturation);
        filters::invertedInputValue = createInverseFunction(filters::inputValue);
        filters::invertedInputLightness = createInverseFunction(filters::inputLightness);
        filters::invertedInputRed = createInverseFunction(filters::inputRed);
        filters::invertedInputGreen = createInverseFunction(filters::inputGreen);
        filters::invertedInputBlue = createInverseFunction(filters::inputBlue);
        filters::invertedInputWhite = createInverseFunction(filters::inputWhite);
        filters::invertedGlobalInput = createInverseFunction(filters::globalInput);

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
    

    void writeOutput() {
        float r, g, b;
        hsvToRgb(hue, saturation, value, r, g, b);        
        float filteredValues[5] = {
            filters::outputRed(filters::globalOutput(r)),
            filters::outputGreen(filters::globalOutput(g)),
            filters::outputBlue(filters::globalOutput(b)),
            filters::outputWhite(filters::globalOutput(white)),
            0.f
        };
        ColorChannels outputValues;
        for (int i=0;i<4;i++)
            outputValues[i] = filteredValues[transistorConnections[i]] * scalling[i];
        ColorChannels periods, phases;
        for (int i=0;i<4;i++)
            periods[i] = addGateLoadingTime(outputValues[i], gateLoadingTime);
        if (phaseMode) {
            float sum = 0;
            for (int i=0;i<4;i++) {
                phases[i] = sum;
                sum += fmod(sum + outputValues[i], 1.f);
            }
        } else {
            for (int i=0;i<4;i++)
                phases[i] = (1.f - outputValues[i]) / 2;
        }
        for (int i=0;i<4;i++) 
            setLedC(
                LED_GPIO_OUTPUTS[i], 
                i, 
                periods[i], 
                phases[i],
                invertOutputs
            );
    }


    void setRGBW(float r, float g, float b, float w) {
        r = filters::globalInput(filters::inputRed(r));
        g = filters::globalInput(filters::inputGreen(g));
        b = filters::globalInput(filters::inputBlue(b));
        w = filters::globalInput(filters::inputWhite(w));
        float h, s, v;
        rgbToHsv(r, g, b, h, s, v);
        white = w;
        value = v;
        if (value != 0) return;
        saturation = s;
        if (saturation == 0) return;
        hue = h;
    }


    void setHSVW(float h, float s, float v, float w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        v = filters::globalInput(filters::inputValue(v));
        w = filters::globalInput(filters::inputWhite(w));
        hue = h;
        saturation = s;
        value = v;
        white = w;
    }


    void setHSLW(float h, float s, float l, float w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        l = filters::globalInput(filters::inputLightness(l));
        w = filters::globalInput(filters::inputWhite(w));
        float hr, sr, vr;
        hslToHsv(h, s, l, hr, sr, vr);
        white = w;
        hue = hr;
        if (l == 1.f || l == 0.f) return;
        saturation = sr;
    }


    void setRaw(const ColorChannels& color) {
        hue = color[0];
        saturation = color[1];
        value = color[2];
        white = color[3];
    }

    void setAuto(const String& colorspace, const ColorChannels& color) {
        if (colorspace == "rgb") return setRGBW(color[0], color[1], color[2], color[3]);
        if (colorspace == "hsv") return setHSVW(color[0], color[1], color[2], color[3]);
        if (colorspace == "hsl") return setHSLW(color[0], color[1], color[2], color[3]);
        if (colorspace == "raw") return setRaw(color);
    }


    ColorChannels getRGBW() {
        float r, g, b, w;
        hsvToRgb(hue, saturation, value, r, g, b);
        r = filters::invertedInputRed(filters::invertedGlobalInput(r));
        g = filters::invertedInputGreen(filters::invertedGlobalInput(g));
        b = filters::invertedInputBlue(filters::invertedGlobalInput(b));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(white));
        return {r, g, b, w};
    }


    ColorChannels getHSVW() {
        float h, s, v, w;
        h = filters::invertedInputHue(filters::invertedGlobalInput(hue));
        s = filters::invertedInputSaturation(filters::invertedGlobalInput(saturation));
        v = filters::invertedInputValue(filters::invertedGlobalInput(value));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(white));
        return {h, s, v, w};
    }


    ColorChannels getHSLW() {
        float h, s, l, w;
        hsvToHsl(hue, saturation, value, h, s, l);
        h = filters::invertedInputHue(filters::invertedGlobalInput(h));
        s = filters::invertedInputSaturation(filters::invertedGlobalInput(s));
        l = filters::invertedInputLightness(filters::invertedGlobalInput(l));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(white));
        return {h, s, l, w};
    }


    ColorChannels getRaw() {
        return {hue, saturation, value, white};
    }


    ColorChannels getAuto(const String& colorspace) {
        if (colorspace == "rgb") return getRGBW();
        if (colorspace == "hsv") return getHSVW();
        if (colorspace == "hsl") return getHSLW();
        if (colorspace == "raw") return getRaw();
        return {0, 0, 0, 0};
    }
    

    String dumpFavoriteColor(const bool useWhite) {
        char buf[16] = {0};
        int size = sprintf(buf, "%01X%02X%02X%02X%02X", 
            useWhite ? 1 : 0,
            (int)round(255*hue),
            (int)round(255*saturation),
            (int)round(255*value),
            (int)round(255*white)
        );
        buf[size] = 0;
        return String(buf);
    }


    ColorChannels decodeFavoriteColor(const String& formattedColor, bool* useWhitePtr) {
        unsigned useWhite, h, s, v, w;
        sscanf(formattedColor.c_str(), "%01X%02X%02X%02X%02X", 
            &useWhite, &h, &s, &v, &w);
        if (useWhitePtr) *useWhitePtr = useWhite;
        return {(float)h / 255.f, (float)s / 255.f, (float)v / 255.f,  (float)w / 255.f};
    }


    ColorChannels favoriteColorPreview(const String& colorspace, const String& formattedColor) {
        ColorChannels color = decodeFavoriteColor(formattedColor, NULL);
        ColorChannels orginal = getRaw();
        setRaw(color);
        ColorChannels preview = getAuto(colorspace);
        setRaw(orginal);
        return preview;
    }


    void applyFavoriteColor(const String& formattedColor) {
        bool useWhite = false;
        ColorChannels color = decodeFavoriteColor(formattedColor, &useWhite);
        if (!useWhite) color[3] = white;
        setRaw(color);
    }

}
