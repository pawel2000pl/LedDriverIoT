#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"

using FloatFunction = std::function<fixed32(fixed32)>;
extern const std::vector<FloatFunction> filterFunctions;

FloatFunction normalizeFunction(FloatFunction fun, fixed32 min_x=0, fixed32 max_x=1);
FloatFunction constrainFunction(FloatFunction fun, fixed32 min_y=0, fixed32 max_y=1);
FloatFunction symFunction(FloatFunction fun);
FloatFunction mixFilterFunctions(const std::vector<fixed32> filters);
FloatFunction createInverseFunction(FloatFunction originalFunction, fixed32 epsilon=std::numeric_limits<fixed32>::min());
FloatFunction periodizeFunction(FloatFunction originalFunction, unsigned count);
