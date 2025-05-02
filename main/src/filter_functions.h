#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"

using FloatFunction = std::function<fraction32(fraction32)>;
extern const std::vector<FloatFunction> filterFunctions;

FloatFunction normalizeFunction(FloatFunction fun, fraction32 min_x=0, fraction32 max_x=1);
FloatFunction constrainFunction(FloatFunction fun, fraction32 min_y=0, fraction32 max_y=1);
FloatFunction symFunction(FloatFunction fun);
FloatFunction mixFilterFunctions(const std::vector<fraction32> filters);
FloatFunction createInverseFunction(FloatFunction originalFunction, fraction32 epsilon=std::numeric_limits<fraction32>::min());
FloatFunction periodizeFunction(FloatFunction originalFunction, unsigned count);
