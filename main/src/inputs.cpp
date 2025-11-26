#include <cmath>
#include "inputs.h"
#include "outputs.h"
#include "conversions.h"
#include "filter_functions.h"

namespace inputs {

    namespace filters {
        ArithmeticFunction inputSaturation;
        ArithmeticFunction inputHue;
        ArithmeticFunction inputValue;
        ArithmeticFunction inputLightness;
        ArithmeticFunction inputRed;
        ArithmeticFunction inputGreen;
        ArithmeticFunction inputBlue;
        ArithmeticFunction inputWhite;
        ArithmeticFunction globalInput;

        ArithmeticFunction invertedInputHue;
        ArithmeticFunction invertedInputSaturation;
        ArithmeticFunction invertedInputValue;
        ArithmeticFunction invertedInputLightness;
        ArithmeticFunction invertedInputRed;
        ArithmeticFunction invertedInputGreen;
        ArithmeticFunction invertedInputBlue;
        ArithmeticFunction invertedInputWhite;
        ArithmeticFunction invertedGlobalInput;

        void clear() {
            inputSaturation = linear_function;
            inputHue = linear_function;
            inputValue = linear_function;
            inputLightness = linear_function;
            inputRed = linear_function;
            inputGreen = linear_function;
            inputBlue = linear_function;
            inputWhite = linear_function;
            globalInput = linear_function;

            invertedInputHue = linear_function;
            invertedInputSaturation = linear_function;
            invertedInputValue = linear_function;
            invertedInputLightness = linear_function;
            invertedInputRed = linear_function;
            invertedInputGreen = linear_function;
            invertedInputBlue = linear_function;
            invertedInputWhite = linear_function;
            invertedGlobalInput = linear_function;
        }
    }


    void updateConfiguration(const JsonVariantConst& configuration) {
        const auto& filters = configuration["filters"];
        const auto& inputFilters = filters["inputFilters"];

        filters::clear();            
        filters::inputSaturation = mixFilterFunctions(toFixedpointVector(inputFilters["saturation"]));
        filters::inputHue = periodizeFunction(mixFilterFunctions(toFixedpointVector(inputFilters["hue"])), 6);
        filters::inputValue = mixFilterFunctions(toFixedpointVector(inputFilters["value"]));
        filters::inputLightness = mixFilterFunctions(toFixedpointVector(inputFilters["lightness"]));
        filters::inputRed = mixFilterFunctions(toFixedpointVector(inputFilters["red"]));
        filters::inputGreen = mixFilterFunctions(toFixedpointVector(inputFilters["green"]));
        filters::inputBlue = mixFilterFunctions(toFixedpointVector(inputFilters["blue"]));
        filters::inputWhite = mixFilterFunctions(toFixedpointVector(inputFilters["white"]));
        filters::globalInput = mixFilterFunctions(toFixedpointVector(filters["globalInputFilters"]));  

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


    void setRGBW(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c w) {
        r = filters::globalInput(filters::inputRed(r));
        g = filters::globalInput(filters::inputGreen(g));
        b = filters::globalInput(filters::inputBlue(b));
        w = filters::globalInput(filters::inputWhite(w));
        fixed32_c h, s, v;
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


    void setHSVW(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        v = filters::globalInput(filters::inputValue(v));
        w = filters::globalInput(filters::inputWhite(w));
        outputs::setColor(h, s, v, w);
    }


    void setHSLW(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        l = filters::globalInput(filters::inputLightness(l));
        w = filters::globalInput(filters::inputWhite(w));
        fixed32_c hr, sr, vr;
        hslToHsv(h, s, l, hr, sr, vr);
        ColorChannels raw = outputs::getColor();
        raw[3] = w;
        raw[2] = vr;
        raw[0] = hr;
        if (l != 1 && l != 0) 
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
        fixed32_c r, g, b, w;
        hsvToRgb(raw[0], raw[1], raw[2], r, g, b);
        r = filters::invertedInputRed(filters::invertedGlobalInput(r));
        g = filters::invertedInputGreen(filters::invertedGlobalInput(g));
        b = filters::invertedInputBlue(filters::invertedGlobalInput(b));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {r, g, b, w};
    }


    ColorChannels getHSVW() {
        ColorChannels raw = outputs::getColor();
        fixed32_c h, s, v, w;
        h = filters::invertedInputHue(filters::invertedGlobalInput(raw[0]));
        s = filters::invertedInputSaturation(filters::invertedGlobalInput(raw[1]));
        v = filters::invertedInputValue(filters::invertedGlobalInput(raw[2]));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {h, s, v, w};
    }


    ColorChannels getHSLW() {
        ColorChannels raw = outputs::getColor();
        fixed32_c h, s, l, w;
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
    

    void byteToHex(unsigned char value, char* ptr) {
        unsigned char lo = value & 0xF;
        unsigned char hi = value >> 4;
        ptr[0] = (hi <= 9) ? ('0' + hi) : (('A' - 10) + hi);
        ptr[1] = (lo <= 9) ? ('0' + lo) : (('A' - 10) + lo);
    }


    String dumpFavoriteColor(const bool useWhite) {
        ColorChannels raw = outputs::getColor();
        char buf[16];
        buf[0] = useWhite ? '1' : '0';
        byteToHex((int)std::round(255*raw[0]), buf+1);
        byteToHex((int)std::round(255*raw[1]), buf+3);
        byteToHex((int)std::round(255*raw[2]), buf+5);
        byteToHex((int)std::round(255*raw[3]), buf+7);
        buf[9] = 0;
        return String(buf);
    }


    unsigned charToDigit(char chr) {
        if (chr <= '9') return chr - '0';
        if (chr >= 'a') return chr - 'a' + 10;
        return chr - 'A' + 10;
    }


    ColorChannels decodeFavoriteColor(const String& formattedColor, bool* useWhitePtr) {
        const char* buf = formattedColor.c_str();
        unsigned useWhite = charToDigit(buf[0]);
        unsigned h = (charToDigit(buf[1]) << 4) | charToDigit(buf[2]);
        unsigned s = (charToDigit(buf[3]) << 4) | charToDigit(buf[4]);
        unsigned v = (charToDigit(buf[5]) << 4) | charToDigit(buf[6]);
        unsigned w = (charToDigit(buf[7]) << 4) | charToDigit(buf[8]);
        if (useWhitePtr) *useWhitePtr = useWhite;
        return {(fixed32_c)h / 255, (fixed32_c)s / 255, (fixed32_c)v / 255,  (fixed32_c)w / 255};
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
