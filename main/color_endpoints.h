#include <stdio.h>
#pragma once

#include <functional>
#include <array>
#include "json.h"

using ColorChannels = std::array<float, 4>;
using ConversionFunction = std::function<void(float, float, float, float&, float&, float&)>;

struct RGBW {
  float red;
  float green;
  float blue;
  float white;

  RGBW(float r=0, float g=0, float b=0, float w=0) {
    red = r;
    green = g;
    blue = b;
    white = w;
  }

  explicit RGBW(ColorChannels channels) {
    red = channels[0];
    green = channels[1];
    blue = channels[2];
    white = channels[3];
  }

  float& operator[] (const int i) {
    return *(&red + i);
  }

  operator String() const {
    const char* format = "RED: %.2f, GREEN: %.2f, BLUE: %.2f, WHITE: %.2f";
    char outputBuf[80];
    sprintf(outputBuf, format, red, green, blue, white);
    return outputBuf;
  }
};

ColorChannels somethingColorCustom(const JsonVariantConst& configuration, ConversionFunction converter, 
  const char* firstGlobalFilterName, const char* middleFilterGroup, const char* lastFilterName,
  bool invertFilters, bool filtersToOutput,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w);

ColorChannels setColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w);
ColorChannels setColorHSV(const JsonVariantConst& configuration, float h, float s, float v, float w);
ColorChannels setColorHSL(const JsonVariantConst& configuration, float h, float s, float l, float w);
ColorChannels setColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w);

ColorChannels getColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w);
ColorChannels getColorHSV(const JsonVariantConst& configuration, float r, float g, float b, float w);
ColorChannels getColorHSL(const JsonVariantConst& configuration, float r, float g, float b, float w);
ColorChannels getColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w);
