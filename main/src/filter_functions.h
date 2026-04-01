#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"
#include "lib/fixedpoint/polyapprox.h"

using ArithmeticFunction = std::function<fixed32_f(fixed32_f)>;
using ArithmeticFloatFunction = std::function<float(float)>;


class MixedFunction {

    public:
        MixedFunction() = default;
        MixedFunction(const MixedFunction&) = default;
        MixedFunction(MixedFunction&&) = default;
        MixedFunction(ArithmeticFunction fun);
        MixedFunction(ArithmeticFloatFunction fun);
        fixed32_f operator()(fixed32_f) const;

        MixedFunction& operator=(const MixedFunction&) = default;
        MixedFunction& operator=(MixedFunction&&) = default;

    private:
        PolyApprox<fixed32_f, 31> approximation;
        fixed32_f fmin;
        fixed32_f fmax;
        fixed32_f minff;
        fixed32_f absfdiff;
};


MixedFunction mixFilterFunctions(const std::vector<float>& filters);
fixed32_f calulcateInversedValue(const ArithmeticFunction& originalFunction, fixed32_f y, fixed32_f epsilon=std::numeric_limits<fixed32_f>::min());
