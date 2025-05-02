#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <cmath>
#endif

#include "conversions.h"

void rgbToHsl(fixed32 r, fixed32 g, fixed32 b, fixed32& h, fixed32& s, fixed32& l) {
    fixed32 maxVal = std::max(std::max(r, g), b);
    fixed32 minVal = std::min(std::min(r, g), b);
    h = 0; s = 0; l = (maxVal + minVal) / 2;

    if (maxVal == minVal) {
        h = 0; 
        s = 0;
    } else {
        fixed32 d = maxVal - minVal;
        s = l > 0.5 ? d / (2 - maxVal - minVal) : d / (maxVal + minVal);

        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

fixed32 hue2rgb(fixed32 p, fixed32 q, fixed32 t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < (fixed32)1/6) return p + (q - p) * 6 * t;
    if (t < (fixed32)1/2) return q;
    if (t < (fixed32)2/3) return p + (q - p) * ((fixed32)2/3 - t) * 6;
    return p;
}

void hslToRgb(fixed32 h, fixed32 s, fixed32 l, fixed32& r, fixed32& g, fixed32& b) {
    if (s == 0) {
        r = g = b = l; 
    } else {
        fixed32 q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        fixed32 p = 2 * l - q;
        r = hue2rgb(p, q, h + (fixed32)1/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - (fixed32)1/3);
    }
}

void rgbToHsv(fixed32 r, fixed32 g, fixed32 b, fixed32& h, fixed32& s, fixed32& v) {
    fixed32 maxVal = std::max(std::max(r, g), b);
    fixed32 minVal = std::min(std::min(r, g), b);
    h = 0, s = 0, v = maxVal;
    fixed32 d = maxVal - minVal;
    s = maxVal == 0 ? (fixed32)0 : d / maxVal;

    if (maxVal == minVal) {
        h = 0; 
    } else {
        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

void hsvToRgb(fixed32 h, fixed32 s, fixed32 v, fixed32& r, fixed32& g, fixed32& b) {
    int i = std::floor(h * 6);
    fixed32 f = h * 6 - i;
    fixed32 p = v * (1 - s);
    fixed32 q = v * (1 - f * s);
    fixed32 t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
}

void hslToHsv(fixed32 h, fixed32 s_l, fixed32 l, fixed32& h_out, fixed32& s_v_out, fixed32& v_out) {
    h_out = h;
    fixed32 v = l + s_l * std::min(l, 1 - l);
    s_v_out = (v == 0) ? (fixed32)0 : 2 * (1 - l / v);
    v_out = v;
}

void hsvToHsl(fixed32 h, fixed32 s_v, fixed32 v, fixed32& h_out, fixed32 & s_l_out, fixed32& l_out) {
    h_out = h;
    fixed32 l = v * (1 - s_v / 2);
    fixed32 m = std::min(l, 1 - l);
    s_l_out = (m == 0) ? (fixed32)0 : (v - l) / m;
    l_out = l;
}

void rgbToRgb(fixed32 r, fixed32 g, fixed32 b, fixed32& outR, fixed32& outG, fixed32& outB) {
    outR = r;
    outG = g;
    outB = b;
}

void hslToHsl(fixed32 h, fixed32 s, fixed32 l, fixed32& outH, fixed32& outS, fixed32& outL) {
    outH = h;
    outS = s;
    outL = l;
}

void hsvToHsv(fixed32 h, fixed32 s, fixed32 v, fixed32& outH, fixed32& outS, fixed32& outV) {
    outH = h;
    outS = s;
    outV = v;
}
