#pragma once
#include "common_types.h"

void rgbToHsl(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c& h, fixed32_c& s, fixed32_c& l);
void hslToRgb(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c& r, fixed32_c& g, fixed32_c& b);
void rgbToHsv(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c& h, fixed32_c& s, fixed32_c& v) ;
void hsvToRgb(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c& r, fixed32_c& g, fixed32_c& b);
void hslToHsv(fixed32_c h, fixed32_c s_l, fixed32_c l, fixed32_c &h_out, fixed32_c &s_v_out, fixed32_c &v_out);
void hsvToHsl(fixed32_c h, fixed32_c s_v, fixed32_c v, fixed32_c &h_out, fixed32_c &s_l_out, fixed32_c &l_out);
void rgbToRgb(fixed32_c r, fixed32_c g, fixed32_c b, fixed32_c& outR, fixed32_c& outG, fixed32_c& outB);
void hsvToHsv(fixed32_c h, fixed32_c s, fixed32_c v, fixed32_c& outH, fixed32_c& outS, fixed32_c& outV);
void hslToHsl(fixed32_c h, fixed32_c s, fixed32_c l, fixed32_c& outH, fixed32_c& outS, fixed32_c& outL);
