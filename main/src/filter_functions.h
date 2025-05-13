#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"

using ArithmeticFunction = std::function<fixed32_f(fixed32_f)>;
extern const std::vector<ArithmeticFunction> filterFunctions;

ArithmeticFunction normalizeFunction(ArithmeticFunction fun, fixed32_f min_x=0, fixed32_f max_x=1);
ArithmeticFunction constrainFunction(ArithmeticFunction fun, fixed32_f min_y=0, fixed32_f max_y=1);
ArithmeticFunction symFunction(ArithmeticFunction fun);
ArithmeticFunction mixFilterFunctions(const std::vector<fixed32_f> filters);
ArithmeticFunction createInverseFunction(ArithmeticFunction originalFunction, fixed32_f epsilon=std::numeric_limits<fixed32_f>::min());
ArithmeticFunction periodizeFunction(ArithmeticFunction originalFunction, unsigned count);
