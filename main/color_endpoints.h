#include <stdio.h>
#pragma once

#include <functional>
#include "json.h"

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

using CallbackColorFunction = std::function<void(const JsonVariantConst&, float, float, float, float)>;
using ConversionFunction = std::function<void(float, float, float, float&, float&, float&)>;

void setColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback);
void setColorHSV(const JsonVariantConst& configuration, float h, float s, float v, float w, const CallbackColorFunction& callback);
void setColorHSL(const JsonVariantConst& configuration, float h, float s, float l, float w, const CallbackColorFunction& callback);
void setColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w, const CallbackColorFunction& callback);

void getColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback);
void getColorHSV(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback);
void getColorHSL(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback);
void getColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w, const CallbackColorFunction& callback);
