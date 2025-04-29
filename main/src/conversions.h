#pragma once
#include "common_types.h"

void rgbToHsl(fixed64 r, fixed64 g, fixed64 b, fixed64& h, fixed64& s, fixed64& l);
void hslToRgb(fixed64 h, fixed64 s, fixed64 l, fixed64& r, fixed64& g, fixed64& b);
void rgbToHsv(fixed64 r, fixed64 g, fixed64 b, fixed64& h, fixed64& s, fixed64& v) ;
void hsvToRgb(fixed64 h, fixed64 s, fixed64 v, fixed64& r, fixed64& g, fixed64& b);
void hslToHsv(fixed64 h, fixed64 s_l, fixed64 l, fixed64 &h_out, fixed64 &s_v_out, fixed64 &v_out);
void hsvToHsl(fixed64 h, fixed64 s_v, fixed64 v, fixed64 &h_out, fixed64 &s_l_out, fixed64 &l_out);
void rgbToRgb(fixed64 r, fixed64 g, fixed64 b, fixed64& outR, fixed64& outG, fixed64& outB);
void hsvToHsv(fixed64 h, fixed64 s, fixed64 v, fixed64& outH, fixed64& outS, fixed64& outV);
void hslToHsl(fixed64 h, fixed64 s, fixed64 l, fixed64& outH, fixed64& outS, fixed64& outL);
