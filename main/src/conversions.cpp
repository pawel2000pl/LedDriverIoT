#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <cmath>
#endif

#include "conversions.h"

void rgbToHsl(fraction32 r, fraction32 g, fraction32 b, fraction32& h, fraction32& s, fraction32& l) {
    fraction32 maxVal = std::max(std::max(r, g), b);
    fraction32 minVal = std::min(std::min(r, g), b);
    h = 0; s = 0; l = (maxVal + minVal) / 2;

    if (maxVal == minVal) {
        h = 0; 
        s = 0;
    } else {
        fraction32 d = maxVal - minVal;
        s = l > 0.5 ? d / (2 - maxVal - minVal) : d / (maxVal + minVal);

        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

fraction32 hue2rgb(fraction32 p, fraction32 q, fraction32 t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < (fraction32)1/6) return p + (q - p) * 6 * t;
    if (t < (fraction32)1/2) return q;
    if (t < (fraction32)2/3) return p + (q - p) * ((fraction32)2/3 - t) * 6;
    return p;
}

void hslToRgb(fraction32 h, fraction32 s, fraction32 l, fraction32& r, fraction32& g, fraction32& b) {
    if (s == 0) {
        r = g = b = l; 
    } else {
        fraction32 q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        fraction32 p = 2 * l - q;
        r = hue2rgb(p, q, h + (fraction32)1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - (fraction32)1/3);
    }
}

void rgbToHsv(fraction32 r, fraction32 g, fraction32 b, fraction32& h, fraction32& s, fraction32& v) {
    fraction32 maxVal = std::max(std::max(r, g), b);
    fraction32 minVal = std::min(std::min(r, g), b);
    h = 0, s = 0, v = maxVal;
    fraction32 d = maxVal - minVal;
    s = maxVal == 0 ? (fraction32)0 : d / maxVal;

    if (maxVal == minVal) {
        h = 0; 
    } else {
        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

void hsvToRgb(fraction32 h, fraction32 s, fraction32 v, fraction32& r, fraction32& g, fraction32& b) {
    int i = std::floor(h * 6);
    fraction32 f = h * 6 - i;
    fraction32 p = v * (1 - s);
    fraction32 q = v * (1 - f * s);
    fraction32 t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
}

void hslToHsv(fraction32 h, fraction32 s_l, fraction32 l, fraction32& h_out, fraction32& s_v_out, fraction32& v_out) {
    h_out = h;
    fraction32 v = l + s_l * std::min(l, 1 - l);
    s_v_out = (v == 0) ? (fraction32)0 : 2 * (1 - l / v);
    v_out = v;
}

void hsvToHsl(fraction32 h, fraction32 s_v, fraction32 v, fraction32& h_out, fraction32 & s_l_out, fraction32& l_out) {
    h_out = h;
    fraction32 l = v * (1 - s_v / 2);
    fraction32 m = std::min(l, 1 - l);
    s_l_out = (m == 0) ? (fraction32)0 : (v - l) / m;
    l_out = l;
}

void rgbToRgb(fraction32 r, fraction32 g, fraction32 b, fraction32& outR, fraction32& outG, fraction32& outB) {
    outR = r;
    outG = g;
    outB = b;
}

void hslToHsl(fraction32 h, fraction32 s, fraction32 l, fraction32& outH, fraction32& outS, fraction32& outL) {
    outH = h;
    outS = s;
    outL = l;
}

void hsvToHsv(fraction32 h, fraction32 s, fraction32 v, fraction32& outH, fraction32& outS, fraction32& outV) {
    outH = h;
    outS = s;
    outV = v;
}
