#pragma once

#include <Arduino.h>
#include <vector>
#include <cmath>
#include <functional>

#include "common_types.h"
#include "lib/fixedpoint/polyapprox.h"

using ArithmeticFunction = std::function<fixed64_f(fixed64_f)>;
using ArithmeticFloatFunction = std::function<float(float)>;


class MixedFunction {

    public:
        MixedFunction() = default;
        MixedFunction(const MixedFunction&) = default;
        MixedFunction(MixedFunction&&) = default;
        MixedFunction(ArithmeticFunction fun);
        MixedFunction(ArithmeticFloatFunction fun);
        fixed64_f operator()(fixed64_f) const;

        MixedFunction& operator=(const MixedFunction&) = default;
        MixedFunction& operator=(MixedFunction&&) = default;

    private:
        PolyApprox<fixed64_f, 31> approximation;
        fixed64_f fmin;
        fixed64_f fmax;
        fixed64_f minff;
        fixed64_f absfdiff;
};


MixedFunction mixFilterFunctions(const std::vector<float>& filters);
fixed64_f calulcateInversedValue(const ArithmeticFunction& originalFunction, fixed64_f y, fixed64_f epsilon=std::numeric_limits<fixed64_f>::min());
