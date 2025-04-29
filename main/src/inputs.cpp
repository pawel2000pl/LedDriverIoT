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


    void setRGBW(fixed64 r, fixed64 g, fixed64 b, fixed64 w) {
        r = filters::globalInput(filters::inputRed(r));
        g = filters::globalInput(filters::inputGreen(g));
        b = filters::globalInput(filters::inputBlue(b));
        w = filters::globalInput(filters::inputWhite(w));
        fixed64 h, s, v;
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


    void setHSVW(fixed64 h, fixed64 s, fixed64 v, fixed64 w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        v = filters::globalInput(filters::inputValue(v));
        w = filters::globalInput(filters::inputWhite(w));
        outputs::setColor(h, s, v, w);
    }


    void setHSLW(fixed64 h, fixed64 s, fixed64 l, fixed64 w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        l = filters::globalInput(filters::inputLightness(l));
        w = filters::globalInput(filters::inputWhite(w));
        fixed64 hr, sr, vr;
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
        fixed64 r, g, b, w;
        hsvToRgb(raw[0], raw[1], raw[2], r, g, b);
        r = filters::invertedInputRed(filters::invertedGlobalInput(r));
        g = filters::invertedInputGreen(filters::invertedGlobalInput(g));
        b = filters::invertedInputBlue(filters::invertedGlobalInput(b));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {r, g, b, w};
    }


    ColorChannels getHSVW() {
        ColorChannels raw = outputs::getColor();
        fixed64 h, s, v, w;
        h = filters::invertedInputHue(filters::invertedGlobalInput(raw[0]));
        s = filters::invertedInputSaturation(filters::invertedGlobalInput(raw[1]));
        v = filters::invertedInputValue(filters::invertedGlobalInput(raw[2]));
        w = filters::invertedInputWhite(filters::invertedGlobalInput(raw[3]));
        return {h, s, v, w};
    }


    ColorChannels getHSLW() {
        ColorChannels raw = outputs::getColor();
        fixed64 h, s, l, w;
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
        return {(fixed64)h / 255.f, (fixed64)s / 255.f, (fixed64)v / 255.f,  (fixed64)w / 255.f};
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
