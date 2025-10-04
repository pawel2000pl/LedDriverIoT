#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <cmath>
#endif

#include "conversions.h"

void rgbToHsl(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c& h, fixed32_c& s, fixed32_c& l) {
    fixed32_c maxVal = std::max(std::max(r, g), b);
    fixed32_c minVal = std::min(std::min(r, g), b);
    h = 0; s = 0; l = (maxVal + minVal) / 2;

    if (maxVal == minVal) {
        h = 0; 
        s = 0;
    } else {
        fixed32_c d = maxVal - minVal;
        s = l > 0.5 ? d / (2 - maxVal - minVal) : d / (maxVal + minVal);

        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

fixed32_c hue2rgb(fixed32_c p, fixed32_c q, fixed32_c t) {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < fixed32_c::fraction(1,6)) return p + (q - p) * 6 * t;
    if (t < fixed32_c::fraction(1,2)) return q;
    if (t < fixed32_c::fraction(2,3)) return p + (q - p) * (fixed32_c::fraction(2,3) - t) * 6;
    return p;
}

void hslToRgb(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c& r, fixed32_c& g, fixed32_c& b) {
    if (s == 0) {
        r = g = b = l; 
    } else {
        fixed32_c q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        fixed32_c p = 2 * l - q;
        r = hue2rgb(p, q, h + fixed32_c::fraction(1,3));
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - fixed32_c::fraction(1,3));
    }
}

void rgbToHsv(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c& h, fixed32_c& s, fixed32_c& v) {
    fixed32_c maxVal = std::max(std::max(r, g), b);
    fixed32_c minVal = std::min(std::min(r, g), b);
    h = 0, s = 0, v = maxVal;
    fixed32_c d = maxVal - minVal;
    s = maxVal == 0 ? (fixed32_c)0 : d / maxVal;

    if (maxVal == minVal) {
        h = 0; 
    } else {
        if (maxVal == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (maxVal == g) h = (b - r) / d + 2;
        else if (maxVal == b) h = (r - g) / d + 4;
        h /= 6;
    }
}

void hsvToRgb(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c& r, fixed32_c& g, fixed32_c& b) {
    int i = std::floor(h * 6);
    fixed32_c vs = v * s;
    fixed32_c f = h * 6 - i;
    fixed32_c vsf = vs * f;
    fixed32_c p = v - vs;
    fixed32_c q = v - vsf;
    fixed32_c t = v - vs - vsf;

    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
}

void hslToHsv(fixed32_c h, fixed32_c s_l, fixed32_c l, fixed32_c& h_out, fixed32_c& s_v_out, fixed32_c& v_out) {
    h_out = h;
    fixed32_c v = l + s_l * std::min(l, 1 - l);
    s_v_out = (v == 0) ? (fixed32_c)0 : 2 * (1 - l / v);
    v_out = v;
}

void hsvToHsl(fixed32_c h, fixed32_c s_v, fixed32_c v, fixed32_c& h_out, fixed32_c & s_l_out, fixed32_c& l_out) {
    h_out = h;
    fixed32_c l = v * (1 - s_v / 2);
    fixed32_c m = std::min(l, 1 - l);
    s_l_out = (m == 0) ? (fixed32_c)0 : (v - l) / m;
    l_out = l;
}

void rgbToRgb(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c& outR, fixed32_c& outG, fixed32_c& outB) {
    outR = r;
    outG = g;
    outB = b;
}

void hslToHsl(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c& outH, fixed32_c& outS, fixed32_c& outL) {
    outH = h;
    outS = s;
    outL = l;
}

void hsvToHsv(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c& outH, fixed32_c& outS, fixed32_c& outV) {
    outH = h;
    outS = s;
    outV = v;
}
