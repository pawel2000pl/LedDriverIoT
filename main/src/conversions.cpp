#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <cmath>
#endif


void rgbToHsl(float r, float g, float b, float& h, float& s, float& l) {
    float maxVal = max(max(r, g), b);
    float minVal = min(min(r, g), b);
    h = 0, s = 0, l = (maxVal + minVal) / 2.0;

    if (maxVal == minVal) {
        h = s = 0;
    } else {
        float d = maxVal - minVal;
        s = l > 0.5 ? d / (2.0 - maxVal - minVal) : d / (maxVal + minVal);

        if (maxVal == r) h = (g - b) / d + (g < b ? 6.0 : 0.0);
        else if (maxVal == g) h = (b - r) / d + 2.0;
        else if (maxVal == b) h = (r - g) / d + 4.0;
        h /= 6.0;
    }
}

float hue2rgb(float p, float q, float t) {
    if (t < 0) t += 1.0;
    if (t > 1) t -= 1.0;
    if (t < 1/6.0) return p + (q - p) * 6.0 * t;
    if (t < 1/2.0) return q;
    if (t < 2/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
    return p;
}

void hslToRgb(float h, float s, float l, float& r, float& g, float& b) {
    if (s == 0) {
        r = g = b = l; 
    } else {
        float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
        float p = 2.0 * l - q;
        r = hue2rgb(p, q, h + 1.0/3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0/3.0);
    }
}

void rgbToHsv(float r, float g, float b, float& h, float& s, float& v) {
    float maxVal = max(max(r, g), b);
    float minVal = min(min(r, g), b);
    h = 0, s = 0, v = maxVal;
    float d = maxVal - minVal;
    s = maxVal == 0 ? 0 : d / maxVal;

    if (maxVal == minVal) {
        h = 0; 
    } else {
        if (maxVal == r) h = (g - b) / d + (g < b ? 6.0 : 0.0);
        else if (maxVal == g) h = (b - r) / d + 2.0;
        else if (maxVal == b) h = (r - g) / d + 4.0;
        h /= 6.0;
    }
}

void hsvToRgb(float h, float s, float v, float& r, float& g, float& b) {
    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
}

void hslToHsv(float h, float s_l, float l, float& h_out, float& s_v_out, float& v_out) {
    h_out = h;
    float v = l + s_l * min(l, 1 - l);
    s_v_out = (v == 0) ? 0 : 2 * (1 - l / v);
    v_out = v;
}

void hsvToHsl(float h, float s_v, float v, float& h_out, float & s_l_out, float& l_out) {
    h_out = h;
    float l = v * (1 - s_v / 2);
    float m = min(l, 1 - l);
    s_l_out = (m == 0) ? 0 : (v - l) / m;
    l_out = l;
}

void rgbToRgb(float r, float g, float b, float& outR, float& outG, float& outB) {
    outR = r;
    outG = g;
    outB = b;
}

void hslToHsl(float h, float s, float l, float& outH, float& outS, float& outL) {
    outH = h;
    outS = s;
    outL = l;
}

void hsvToHsv(float h, float s, float v, float& outH, float& outS, float& outV) {
    outH = h;
    outS = s;
    outV = v;
}
