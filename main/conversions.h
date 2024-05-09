#pragma once

void rgbToHsl(float r, float g, float b, float& h, float& s, float& l);
void hslToRgb(float h, float s, float l, float& r, float& g, float& b);
void rgbToHsv(float r, float g, float b, float& h, float& s, float& v) ;
void hsvToRgb(float h, float s, float v, float& r, float& g, float& b);
void rgbToRgb(float r, float g, float b, float& outR, float& outG, float& outB);
