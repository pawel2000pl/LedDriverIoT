#include <functional>
#include "color_endpoints.h"
#include "functions.h"
#include "conversions.h"

using FilterFilter = std::function<FloatFunction(FloatFunction)>;

FloatFunction returnSameFloatFunction(FloatFunction originalFunction) {
  return originalFunction;
}

float returnSameFloat(float x) {
  return x;
}

void somethingColorCustom(const JsonVariantConst& configuration, ConversionFunction converter, 
  const char* firstGlobalFilterName, const char* middleFilterGroup, const char* lastFilterName,
  bool invertFilters, bool filtersToOutput,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w,
  const CallbackColorFunction& callback) {

  const JsonVariantConst& filters = configuration["filters"];
  const JsonVariantConst& inputFilters = filters[middleFilterGroup];

  FilterFilter invertor = invertFilters ? createInverseFunction : returnSameFloatFunction;

  FloatFunction filter_c1 = invertor(mixFilterFunctions(inputFilters[channel1]));
  FloatFunction filter_c2 = invertor(mixFilterFunctions(inputFilters[channel2]));
  FloatFunction filter_c3 = invertor(mixFilterFunctions(inputFilters[channel3]));
  FloatFunction filter_w = invertor(mixFilterFunctions(inputFilters["white"]));
  FloatFunction filter_global_1 = firstGlobalFilterName ? invertor(mixFilterFunctions(filters[firstGlobalFilterName])) : returnSameFloat;
  FloatFunction filter_global_2 = lastFilterName ? invertor(mixFilterFunctions(filters[lastFilterName])) : returnSameFloat;

  float oc1, oc2, oc3, ow;

  if (!filtersToOutput) {
    c1 = filter_global_2(filter_c1(filter_global_1(c1))); 
    c2 = filter_global_2(filter_c2(filter_global_1(c2))); 
    c3 = filter_global_2(filter_c3(filter_global_1(c3)));
  }

  converter(c1, c2, c3, oc1, oc2, oc3);
  ow = filter_global_2(filter_w(filter_global_1(w)));

  if (filtersToOutput) {
    oc1 = filter_global_2(filter_c1(filter_global_1(oc1))); 
    oc2 = filter_global_2(filter_c2(filter_global_1(oc2))); 
    oc3 = filter_global_2(filter_c3(filter_global_1(oc3)));
  }

  callback(configuration, oc1, oc2, oc3, ow);
}


void setColorCustom(const JsonVariantConst& configuration, ConversionFunction converter,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w,
  const CallbackColorFunction& callback) {
  somethingColorCustom(
    configuration, converter, NULL, "inputFilters", "globalInputFilters",
    false, false,
    channel1, channel2, channel3,
    c1, c2, c3, w,
    callback
  );
}

void getColorCustom(const JsonVariantConst& configuration, ConversionFunction converter,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w,
  const CallbackColorFunction& callback) {
  somethingColorCustom(
    configuration, converter, "globalInputFilters", "inputFilters", NULL,
    true, true,
    channel1, channel2, channel3,
    c1, c2, c3, w,
    callback
  );
}


void setColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback) {
  setColorCustom(
    configuration,
    rgbToRgb,
    "red", "green", "blue",
    r, g, b, w,
    callback
  );
}

void setColorHSV(const JsonVariantConst& configuration, float h, float s, float v, float w, const CallbackColorFunction& callback) {
  setColorCustom(
    configuration,
    hsvToRgb,
    "hue", "saturation", "value",
    h, s, v, w,
    callback
  );
}

void setColorHSL(const JsonVariantConst& configuration, float h, float s, float l, float w, const CallbackColorFunction& callback) {
  setColorCustom(
    configuration,
    hslToRgb,
    "hue", "saturation", "lightness",
    h, s, l, w,
    callback
  );
}

void setColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w, const CallbackColorFunction& callback) {
  if (colorspace == "rgb") setColorRGB(configuration, c1, c2, c3, w, callback);
  if (colorspace == "hsl") setColorHSL(configuration, c1, c2, c3, w, callback);
  if (colorspace == "hsv") setColorHSV(configuration, c1, c2, c3, w, callback);
}


void getColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback) {
  getColorCustom(
    configuration,
    rgbToRgb,
    "red", "green", "blue",
    r, g, b, w,
    callback
  );
}

void getColorHSV(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback) {
  getColorCustom(
    configuration,
    rgbToHsv,
    "hue", "saturation", "value",
    r, g, b, w,
    callback
  );
}

void getColorHSL(const JsonVariantConst& configuration, float r, float g, float b, float w, const CallbackColorFunction& callback) {
  getColorCustom(
    configuration,
    rgbToHsl,
    "hue", "saturation", "lightness",
    r, g, b, w,
    callback
  );
}

void getColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w, const CallbackColorFunction& callback) {
  if (colorspace == "rgb") getColorRGB(configuration, c1, c2, c3, w, callback);
  if (colorspace == "hsl") getColorHSL(configuration, c1, c2, c3, w, callback);
  if (colorspace == "hsv") getColorHSV(configuration, c1, c2, c3, w, callback);
}

