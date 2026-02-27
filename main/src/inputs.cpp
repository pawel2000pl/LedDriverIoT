#include <cmath>
#include "inputs.h"
#include "outputs.h"
#include "conversions.h"
#include "filter_functions.h"

namespace inputs {

    SourceControl source_control = scKnobs;

    namespace filters {
        MixedFunction inputSaturation;
        MixedFunction inputHueBasic;
        MixedFunction inputValue;
        MixedFunction inputLightness;
        MixedFunction inputRed;
        MixedFunction inputGreen;
        MixedFunction inputBlue;
        MixedFunction inputWhite;
        MixedFunction globalInput;

        fixed32_f inputHue(fixed32_f x) {
            constexpr const unsigned count = 6;
            constexpr const fixed32_f frac = fixed32_f(1) / count;
            unsigned i = std::floor(x * count);
            fixed32_f ifrac = i * frac;
            fixed32_f xf = (x - ifrac) * count;
            fixed32_f rp = (i & 1) ? fixed32_f(1) - inputHueBasic(fixed32_f(1) - xf) : inputHueBasic(xf);
            return rp * frac + ifrac;
        }

        fixed32_f invertedInputHue(fixed32_f y) {
            return calulcateInversedValue(inputHue, y);
        }

        fixed32_f invertedInputSaturation(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputSaturation(y);}, y);
        }

        fixed32_f invertedInputValue(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputValue(y);}, y);
        }

        fixed32_f invertedInputLightness(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputLightness(y);}, y);
        }

        fixed32_f invertedInputRed(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputRed(y);}, y);
        }

        fixed32_f invertedInputGreen(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputGreen(y);}, y);
        }

        fixed32_f invertedInputBlue(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputBlue(y);}, y);
        }

        fixed32_f invertedInputWhite(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return inputWhite(y);}, y);
        }

        fixed32_f invertedGlobalInput(fixed32_f y) {
            return calulcateInversedValue([&](fixed32_f y){return globalInput(y);}, y);
        }

    }


    void updateConfiguration(const JsonVariantConst configuration) {
        const auto filters = configuration["filters"];
        const auto inputFilters = filters["inputFilters"];
        
        filters::inputSaturation = mixFilterFunctions(toFixedpointVector(inputFilters["saturation"]));
        filters::inputHueBasic = mixFilterFunctions(toFixedpointVector(inputFilters["hue"]));
        filters::inputValue = mixFilterFunctions(toFixedpointVector(inputFilters["value"]));
        filters::inputLightness = mixFilterFunctions(toFixedpointVector(inputFilters["lightness"]));
        filters::inputRed = mixFilterFunctions(toFixedpointVector(inputFilters["red"]));
        filters::inputGreen = mixFilterFunctions(toFixedpointVector(inputFilters["green"]));
        filters::inputBlue = mixFilterFunctions(toFixedpointVector(inputFilters["blue"]));
        filters::inputWhite = mixFilterFunctions(toFixedpointVector(inputFilters["white"]));
        filters::globalInput = mixFilterFunctions(toFixedpointVector(filters["globalInputFilters"]));  
    }


    ColorChannels prepareRGBW(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c w) {
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
        return raw;
    }


    void setRGBW(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c w) {
        outputs::setColor(prepareRGBW(r, g, b, w));
    }


    ColorChannels prepareHSVW(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w) {
        h = filters::globalInput(filters::inputHue(h));
        s = filters::globalInput(filters::inputSaturation(s));
        v = filters::globalInput(filters::inputValue(v));
        w = filters::globalInput(filters::inputWhite(w));
        return {h, s, v, w};
    }


    void setHSVW(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c w) {
        outputs::setColor(prepareHSVW(h, s, v, w));
    }


    ColorChannels prepareHSLW(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c w) {
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
        return raw;
    }


    void setHSLW(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c w) {
        outputs::setColor(prepareHSLW(h, s, l, w));
    }


    ColorChannels prepareAuto(const String& colorspace, const ColorChannels& color) {
        if (colorspace == "rgb") return prepareRGBW(color[0], color[1], color[2], color[3]);
        if (colorspace == "hsv") return prepareHSVW(color[0], color[1], color[2], color[3]);
        if (colorspace == "hsl") return prepareHSLW(color[0], color[1], color[2], color[3]);
        return color;
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
