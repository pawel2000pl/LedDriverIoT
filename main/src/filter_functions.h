#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"

using FloatFunction = std::function<fixed64(fixed64)>;
extern const std::vector<FloatFunction> filterFunctions;

FloatFunction normalizeFunction(FloatFunction fun, fixed64 min_x=0, fixed64 max_x=1);
FloatFunction constrainFunction(FloatFunction fun, fixed64 min_y=0, fixed64 max_y=1);
FloatFunction symFunction(FloatFunction fun);
FloatFunction mixFilterFunctions(const std::vector<fixed64> filters);
FloatFunction createInverseFunction(FloatFunction originalFunction, fixed64 epsilon=1e-6f);
FloatFunction periodizeFunction(FloatFunction originalFunction, unsigned count);
