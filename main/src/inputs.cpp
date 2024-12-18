#include <cmath>
#include "inputs.h"
#include "outputs.h"
#include "conversions.h"
#include "filter_functions.h"

namespace inputs {

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


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto& filters = configuration["filters"];
        const auto& inputFilters = filters["inputFilters"];
                
        filters::inputSaturation = mixFilterFunctions(toFloatVector(inputFilters["saturation"]));
        filters::inputHue = periodizeFunction(mixFilterFunctions(toFloatVector(inputFilters["hue"])), 6);
        filters::inputValue = mixFilterFunctions(toFloatVector(inputFilters["value"]));
        filters::inputLightness = mixFilterFunctions(toFloatVector(inputFilters["lightness"]));
        filters::inputRed = mixFilterFunctions(toFloatVector(inputFilters["red"]));
        filters::inputGreen = mixFilterFunctions(toFloatVector(inputFilters["green"]));
        filters::inputBlue = mixFilterFunctions(toFloatVector(inputFilters["blue"]));
        filters::inputWhite = mixFilterFunctions(toFloatVector(inputFilters["white"]));
        filters::globalInput = mixFilterFunctions(toFloatVector(filters["globalInputFilters"]));  

        filters::invertedInputHue = createInverseFunction(filters::inputHue);
        filters::invertedInputSaturation = createInverseFunction(filters::inputSaturation);
        filters::invertedInputValue = createInverseFunction(filters::inputValue);
        filters::invertedInputLightness = createInverseFunction(filters::inputLightness);
        filters::invertedInputRed = createInverseFunction(filters::inputRed);
        filters::invertedInputGreen = createInverseFunction(filters::inputGreen);
        filters::invertedInputBlue = createInverseFunction(filters::inputBlue);
        filters::invertedInputWhite = createInverseFunction(filters::inputWhite);
        filters::invertedGlobalInput = createInverseFunction(filters::globalInput);
    }


    void setRGBW(float r, float g, float b, float w) {
        r = filters::globalInput(filters::inputRed(r));
        g = filters::globalInput(filters::inputGreen(g));
        b = filters::globalInput(filters::inputBlue(b));
        w = filters::globalInput(filters::inputWhite(w));
        float h, s, v;
        rgbToHsv(r, g, b, h, s, v);
        ColorChannels raw = outputs::getColor();
        raw[3] = w;
        raw[2] = v;
        if (v != 0) {
            raw[1] = s;
            if (s != 0)
                raw[0] = h;
        }
        outputs::setColor(raw);
    }


    void setHSVW(float h, float s, float v, float w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        v = filters::globalInput(filters::inputValue(v));
        w = filters::globalInput(filters::inputWhite(w));
        outputs::setColor(h, s, v, w);
    }


    void setHSLW(float h, float s, float l, float w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        l = filters::globalInput(filters::inputLightness(l));
        w = filters::globalInput(filters::inputWhite(w));
        float hr, sr, vr;
        hslToHsv(h, s, l, hr, sr, vr);
        ColorChannels raw = outputs::getColor();
        raw[3] = w;
        raw[2] = vr;
        raw[0] = hr;
        if (l != 1.f && l != 0.f) 
            raw[1] = sr;
        outputs::setColor(raw);
    }


    void setAuto(const String& colorspace, const ColorChannels& color) {
        if (colorspace == "rgb") return setRGBW(color[0], color[1], color[2], color[3]);
        if (colorspace == "hsv") return setHSVW(color[0], color[1], color[2], color[3]);
        if (colorspace == "hsl") return setHSLW(color[0], color[1], color[2], color[3]);
        if (colorspace == "raw") return outputs::setColor(color);
    }


    ColorChannels getRGBW() {
        ColorChannels raw = outputs::getColor();
        float r, g, b, w;
        hsvToRgb(raw[0], raw[1], raw[2], r, g, b);
        r = filters::invertedInputRed(filters::invertedGlobalInput(r));
        g = filters::invertedInputGreen(filters::invertedGlobalInput(g));
        b = filters::invertedInputBlue(filters::invertedGlobalInput(b));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {r, g, b, w};
    }


    ColorChannels getHSVW() {
        ColorChannels raw = outputs::getColor();
        float h, s, v, w;
        h = filters::invertedInputHue(filters::invertedGlobalInput(raw[0]));
        s = filters::invertedInputSaturation(filters::invertedGlobalInput(raw[1]));
        v = filters::invertedInputValue(filters::invertedGlobalInput(raw[2]));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {h, s, v, w};
    }


    ColorChannels getHSLW() {
        ColorChannels raw = outputs::getColor();
        float h, s, l, w;
        hsvToHsl(raw[0], raw[1], raw[2], h, s, l);
        h = filters::invertedInputHue(filters::invertedGlobalInput(h));
        s = filters::invertedInputSaturation(filters::invertedGlobalInput(s));
        l = filters::invertedInputLightness(filters::invertedGlobalInput(l));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {h, s, l, w};
    }


    ColorChannels getAuto(const String& colorspace) {
        if (colorspace == "rgb") return getRGBW();
        if (colorspace == "hsv") return getHSVW();
        if (colorspace == "hsl") return getHSLW();
        if (colorspace == "raw") return outputs::getColor();
        return {0, 0, 0, 0};
    }
    

    String dumpFavoriteColor(const bool useWhite) {
        ColorChannels raw = outputs::getColor();
        char buf[16] = {0};
        int size = sprintf(buf, "%01X%02X%02X%02X%02X", 
            useWhite ? 1 : 0,
            (int)round(255*raw[0]),
            (int)round(255*raw[1]),
            (int)round(255*raw[2]),
            (int)round(255*raw[3])
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
        ColorChannels orginal = outputs::getColor();
        outputs::setColor(color);
        ColorChannels preview = getAuto(colorspace);
        outputs::setColor(orginal);
        return preview;
    }


    void applyFavoriteColor(const String& formattedColor) {
        bool useWhite = false;
        ColorChannels color = decodeFavoriteColor(formattedColor, &useWhite);
        if (!useWhite) {
            ColorChannels raw = outputs::getColor();
            color[3] = raw[3];
        }
        outputs::setColor(color);
    }

}
