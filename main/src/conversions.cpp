#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <cmath>
#endif

#include "conversions.h"

void rgbToHsl(fixed64 r, fixed64 g, fixed64 b, fixed64& h, fixed64& s, fixed64& l) {
    fixed64 maxVal = std::max(std::max(r, g), b);
    fixed64 minVal = std::min(std::min(r, g), b);
    h = 0; s = 0; l = (maxVal + minVal) / 2;

    if (maxVal == minVal) {
        h = 0; 
        s = 0;
    } else {
        fixed64 d = maxVal - minVal;
        s = l > 0.5 ? d / (2 - maxVal - minVal) : d / (maxVal + minVal);

        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

fixed64 hue2rgb(fixed64 p, fixed64 q, fixed64 t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < fixed64::fraction(1,6)) return p + (q - p) * 6 * t;
    if (t < fixed64::fraction(1,2)) return q;
    if (t < fixed64::fraction(2,3)) return p + (q - p) * (fixed64::fraction(2,3) - t) * 6;
    return p;
}

void hslToRgb(fixed64 h, fixed64 s, fixed64 l, fixed64& r, fixed64& g, fixed64& b) {
    if (s == 0) {
        r = g = b = l; 
    } else {
        fixed64 q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        fixed64 p = 2 * l - q;
        r = hue2rgb(p, q, h + fixed64::fraction(1,3));
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - fixed64::fraction(1,3));
    }
}

void rgbToHsv(fixed64 r, fixed64 g, fixed64 b, fixed64& h, fixed64& s, fixed64& v) {
    fixed64 maxVal = std::max(std::max(r, g), b);
    fixed64 minVal = std::min(std::min(r, g), b);
    h = 0, s = 0, v = maxVal;
    fixed64 d = maxVal - minVal;
    s = maxVal == 0 ? (fixed64)0 : d / maxVal;

    if (maxVal == minVal) {
        h = 0; 
    } else {
        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

void hsvToRgb(fixed64 h, fixed64 s, fixed64 v, fixed64& r, fixed64& g, fixed64& b) {
    int i = std::floor(h * 6);
    fixed64 f = h * 6 - i;
    fixed64 p = v * (1 - s);
    fixed64 q = v * (1 - f * s);
    fixed64 t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
}

void hslToHsv(fixed64 h, fixed64 s_l, fixed64 l, fixed64& h_out, fixed64& s_v_out, fixed64& v_out) {
    h_out = h;
    fixed64 v = l + s_l * std::min(l, 1 - l);
    s_v_out = (v == 0) ? (fixed64)0 : 2 * (1 - l / v);
    v_out = v;
}

void hsvToHsl(fixed64 h, fixed64 s_v, fixed64 v, fixed64& h_out, fixed64 & s_l_out, fixed64& l_out) {
    h_out = h;
    fixed64 l = v * (1 - s_v / 2);
    fixed64 m = std::min(l, 1 - l);
    s_l_out = (m == 0) ? (fixed64)0 : (v - l) / m;
    l_out = l;
}

void rgbToRgb(fixed64 r, fixed64 g, fixed64 b, fixed64& outR, fixed64& outG, fixed64& outB) {
    outR = r;
    outG = g;
    outB = b;
}

void hslToHsl(fixed64 h, fixed64 s, fixed64 l, fixed64& outH, fixed64& outS, fixed64& outL) {
    outH = h;
    outS = s;
    outL = l;
}

void hsvToHsv(fixed64 h, fixed64 s, fixed64 v, fixed64& outH, fixed64& outS, fixed64& outV) {
    outH = h;
    outS = s;
    outV = v;
}
