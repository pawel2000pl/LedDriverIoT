#include "filter_functions.h"
#include "taylormath.h"

FloatFunction normalizeFunction(FloatFunction fun, fixed32 min_x, fixed32 max_x) {
	const fixed32 fmin = fun(min_x);
	const fixed32 fmax = fun(max_x);
	const fixed32 x_diff = max_x-min_x;
	const fixed32 minff = std::min(fmin, fmax);
	const fixed32 absfdiff = std::abs(fmax-fmin);
	return [=](fixed32 x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}

FloatFunction constrainFunction(FloatFunction fun, fixed32 min_y, fixed32 max_y) {
	return [=](fixed32 x) {
		fixed32 y = fun(x);
		return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
	};
}

FloatFunction symFunction(FloatFunction fun) {
	return [=](fixed32 x) { return 1-fun(1-x); };
}


const std::vector<FloatFunction> filterFunctions = {
	[](fixed32 x) {return x; },
	[](fixed32 x) {return x*x; },
	[](fixed32 x) {return taylor::sqrt<fixed32>(x); },
	normalizeFunction([](fixed32 x) { return taylor::exp<fixed32>(M_PI*(x-1)); }),
	normalizeFunction([](fixed32 x) { return taylor::asin<fixed32>(x*2-1); }),
	normalizeFunction([](fixed32 x) { return taylor::cos<fixed32>((x - 1) * M_PI); }),
	symFunction([](fixed32 x) {return x*x; }),
	symFunction([](fixed32 x) {return taylor::sqrt<fixed32>(x); }),
	normalizeFunction(symFunction([](fixed32 x) { return taylor::exp<fixed32>(M_PI*(x-1)); }))
};
const std::vector<FloatFunction>* filterFunctionsPtr = &filterFunctions;
const unsigned filterFunctionsCount = filterFunctions.size();

FloatFunction mixFilterFunctions(std::vector<fixed32> filters) {
	while (filters.size() < filterFunctionsCount) filters.push_back(0);
	return constrainFunction(normalizeFunction([=](fixed32 x) { 
		fixed32 sum = 0;
		for (int i=0;i<filterFunctionsCount;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctionsPtr->at(i)(x);
		return sum;
	}));
}

FloatFunction createInverseFunction(FloatFunction originalFunction, fixed32 epsilon) {
		const bool minus = originalFunction(0) > originalFunction(1);    
		return [=](fixed32 y) {
				fixed32 left = 0;
				fixed32 right = 1;
				fixed32 prev_mid = -1;
				if (minus) y = -y;
				while (right - left > epsilon) {
						const fixed32 mid = (left + right) / 2;
						if (mid == prev_mid) break;
						prev_mid = mid;
						fixed32 value = originalFunction(mid);
						if (minus) value = -value;
						if (value < y)
							left = mid;
						else
							right = mid;
				}
				return left; 
		};
}

FloatFunction periodizeFunction(FloatFunction originalFunction, unsigned count) {
	fixed32 frac = fixed32(1) / count;
	return [=](fixed32 x) {
		unsigned i = std::floor(x * count);
		fixed32 ifrac = i * frac;
		fixed32 xf = (x - ifrac) * count;
		fixed32 rp = (i & 1) ? fixed32(1) - originalFunction(fixed32(1) - xf) : originalFunction(xf);
		return rp * frac + ifrac;
	};
}
