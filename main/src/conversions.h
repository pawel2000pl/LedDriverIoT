#pragma once
#include "common_types.h"

void rgbToHsl(fixed32 r, fixed32 g, fixed32 b, fixed32& h, fixed32& s, fixed32& l);
void hslToRgb(fixed32 h, fixed32 s, fixed32 l, fixed32& r, fixed32& g, fixed32& b);
void rgbToHsv(fixed32 r, fixed32 g, fixed32 b, fixed32& h, fixed32& s, fixed32& v) ;
void hsvToRgb(fixed32 h, fixed32 s, fixed32 v, fixed32& r, fixed32& g, fixed32& b);
void hslToHsv(fixed32 h, fixed32 s_l, fixed32 l, fixed32 &h_out, fixed32 &s_v_out, fixed32 &v_out);
void hsvToHsl(fixed32 h, fixed32 s_v, fixed32 v, fixed32 &h_out, fixed32 &s_l_out, fixed32 &l_out);
void rgbToRgb(fixed32 r, fixed32 g, fixed32 b, fixed32& outR, fixed32& outG, fixed32& outB);
void hsvToHsv(fixed32 h, fixed32 s, fixed32 v, fixed32& outH, fixed32& outS, fixed32& outV);
void hslToHsl(fixed32 h, fixed32 s, fixed32 l, fixed32& outH, fixed32& outS, fixed32& outL);
