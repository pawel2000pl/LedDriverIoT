#pragma once
#include "common_types.h"

void rgbToHsl(fraction32 r, fraction32 g, fraction32 b, fraction32& h, fraction32& s, fraction32& l);
void hslToRgb(fraction32 h, fraction32 s, fraction32 l, fraction32& r, fraction32& g, fraction32& b);
void rgbToHsv(fraction32 r, fraction32 g, fraction32 b, fraction32& h, fraction32& s, fraction32& v) ;
void hsvToRgb(fraction32 h, fraction32 s, fraction32 v, fraction32& r, fraction32& g, fraction32& b);
void hslToHsv(fraction32 h, fraction32 s_l, fraction32 l, fraction32 &h_out, fraction32 &s_v_out, fraction32 &v_out);
void hsvToHsl(fraction32 h, fraction32 s_v, fraction32 v, fraction32 &h_out, fraction32 &s_l_out, fraction32 &l_out);
void rgbToRgb(fraction32 r, fraction32 g, fraction32 b, fraction32& outR, fraction32& outG, fraction32& outB);
void hsvToHsv(fraction32 h, fraction32 s, fraction32 v, fraction32& outH, fraction32& outS, fraction32& outV);
void hslToHsl(fraction32 h, fraction32 s, fraction32 l, fraction32& outH, fraction32& outS, fraction32& outL);
