#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>


using FloatFunction = std::function<float(float)>;
extern const std::vector<FloatFunction> filterFunctions;

FloatFunction normalizeFunction(FloatFunction fun, float min_x=0, float max_x=1);
FloatFunction constrainFunction(FloatFunction fun, float min_y=0, float max_y=1);
FloatFunction symFunction(FloatFunction fun);
FloatFunction mixFilterFunctions(const std::vector<float> filters);
FloatFunction createInverseFunction(FloatFunction originalFunction, float epsilon=1e-6f);
FloatFunction periodizeFunction(FloatFunction originalFunction, unsigned count);
