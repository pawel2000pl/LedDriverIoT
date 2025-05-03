#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"

using ArithmeticFunction = std::function<fixed64(fixed64)>;
extern const std::vector<ArithmeticFunction> filterFunctions;

ArithmeticFunction normalizeFunction(ArithmeticFunction fun, fixed64 min_x=0, fixed64 max_x=1);
ArithmeticFunction constrainFunction(ArithmeticFunction fun, fixed64 min_y=0, fixed64 max_y=1);
ArithmeticFunction symFunction(ArithmeticFunction fun);
ArithmeticFunction mixFilterFunctions(const std::vector<fixed64> filters);
ArithmeticFunction createInverseFunction(ArithmeticFunction originalFunction, fixed64 epsilon=std::numeric_limits<fixed64>::min());
ArithmeticFunction periodizeFunction(ArithmeticFunction originalFunction, unsigned count);
