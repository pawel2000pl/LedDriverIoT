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

ColorChannels somethingColorCustom(const JsonVariantConst& configuration, ConversionFunction converter, 
  const char* firstGlobalFilterName, const char* middleFilterGroup, const char* lastFilterName,
  bool invertFilters, bool filtersToOutput,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w) {

  const JsonVariantConst& filters = configuration["filters"];
  const JsonVariantConst& middleFilters = filters[middleFilterGroup];

  FilterFilter invertor = invertFilters ? createInverseFunction : returnSameFloatFunction;

  FloatFunction filter_c1 = invertor(mixFilterFunctions(middleFilters[channel1]));
  FloatFunction filter_c2 = invertor(mixFilterFunctions(middleFilters[channel2]));
  FloatFunction filter_c3 = invertor(mixFilterFunctions(middleFilters[channel3]));
  FloatFunction filter_w = invertor(mixFilterFunctions(middleFilters["white"]));
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

  return {oc1, oc2, oc3, ow};
}


ColorChannels setColorCustom(const JsonVariantConst& configuration, ConversionFunction converter,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w) {
  return somethingColorCustom(
    configuration, converter, NULL, "inputFilters", "globalInputFilters",
    false, false,
    channel1, channel2, channel3,
    c1, c2, c3, w
  );
}

ColorChannels getColorCustom(const JsonVariantConst& configuration, ConversionFunction converter,
  const char* channel1, const char* channel2, const char* channel3, 
  float c1, float c2, float c3, float w) {
  return somethingColorCustom(
    configuration, converter, "globalInputFilters", "inputFilters", NULL,
    true, true,
    channel1, channel2, channel3,
    c1, c2, c3, w
  );
}


ColorChannels setColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w) {
  return setColorCustom(
    configuration,
    rgbToRgb,
    "red", "green", "blue",
    r, g, b, w
  );
}

ColorChannels setColorHSV(const JsonVariantConst& configuration, float h, float s, float v, float w) {
  return setColorCustom(
    configuration,
    hsvToRgb,
    "hue", "saturation", "value",
    h, s, v, w
  );
}

ColorChannels setColorHSL(const JsonVariantConst& configuration, float h, float s, float l, float w) {
  return setColorCustom(
    configuration,
    hslToRgb,
    "hue", "saturation", "lightness",
    h, s, l, w
  );
}

ColorChannels setColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w) {
  if (colorspace == "rgb") return setColorRGB(configuration, c1, c2, c3, w);
  if (colorspace == "hsl") return setColorHSL(configuration, c1, c2, c3, w);
  if (colorspace == "hsv") return setColorHSV(configuration, c1, c2, c3, w);
  return {0, 0, 0, 0};
}


ColorChannels getColorRGB(const JsonVariantConst& configuration, float r, float g, float b, float w) {
  return getColorCustom(
    configuration,
    rgbToRgb,
    "red", "green", "blue",
    r, g, b, w
  );
}

ColorChannels getColorHSV(const JsonVariantConst& configuration, float r, float g, float b, float w) {
  return getColorCustom(
    configuration,
    rgbToHsv,
    "hue", "saturation", "value",
    r, g, b, w
  );
}

ColorChannels getColorHSL(const JsonVariantConst& configuration, float r, float g, float b, float w) {
  return getColorCustom(
    configuration,
    rgbToHsl,
    "hue", "saturation", "lightness",
    r, g, b, w
  );
}

ColorChannels getColorAuto(const JsonVariantConst& configuration, String colorspace, float c1, float c2, float c3, float w) {
  if (colorspace == "rgb") return getColorRGB(configuration, c1, c2, c3, w);
  if (colorspace == "hsl") return getColorHSL(configuration, c1, c2, c3, w);
  if (colorspace == "hsv") return getColorHSV(configuration, c1, c2, c3, w);
  return {0, 0, 0, 0};
}

