#pragma once

void rgbToHsl(float r, float g, float b, float& h, float& s, float& l);
void hslToRgb(float h, float s, float l, float& r, float& g, float& b);
void rgbToHsv(float r, float g, float b, float& h, float& s, float& v) ;
void hsvToRgb(float h, float s, float v, float& r, float& g, float& b);
void hslToHsv(float h, float s_l, float l, float &h_out, float &s_v_out, float &v_out);
void hsvToHsl(float h, float s_v, float v, float &h_out, float &s_l_out, float &l_out);
void rgbToRgb(float r, float g, float b, float& outR, float& outG, float& outB);
void hsvToHsv(float h, float s, float v, float& outH, float& outS, float& outV);
void hslToHsl(float h, float s, float l, float& outH, float& outS, float& outL);
