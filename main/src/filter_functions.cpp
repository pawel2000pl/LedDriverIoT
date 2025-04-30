#include "filter_functions.h"
#include "taylormath.h"

FloatFunction normalizeFunction(FloatFunction fun, fixed64 min_x, fixed64 max_x) {
	const fixed64 fmin = fun(min_x);
	const fixed64 fmax = fun(max_x);
	const fixed64 x_diff = max_x-min_x;
	const fixed64 minff = min(fmin, fmax);
	const fixed64 absfdiff = abs(fmax-fmin);
	return [=](fixed64 x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}

FloatFunction constrainFunction(FloatFunction fun, fixed64 min_y, fixed64 max_y) {
	return [=](fixed64 x) {
		fixed64 y = fun(x);
		return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
	};
}

FloatFunction symFunction(FloatFunction fun) {
	return [=](fixed64 x) { return 1-fun(1-x); };
}


const std::vector<FloatFunction> filterFunctions = {
	[](fixed64 x) {return x; },
	[](fixed64 x) {return x*x; },
	[](fixed64 x) {return taylor::sqrt<fixed64>(x); },
	normalizeFunction([](fixed64 x) { return taylor::exp<fixed64>(M_PI*(x-1)); }),
	normalizeFunction([](fixed64 x) { return taylor::asin<fixed64>(x*2-1); }),
	normalizeFunction([](fixed64 x) { return taylor::cos<fixed64>((x - 1) * M_PI); }),
	symFunction([](fixed64 x) {return x*x; }),
	symFunction([](fixed64 x) {return taylor::sqrt<fixed64>(x); }),
	normalizeFunction(symFunction([](fixed64 x) { return taylor::exp<fixed64>(M_PI*(x-1)); }))
};
const std::vector<FloatFunction>* filterFunctionsPtr = &filterFunctions;
const unsigned filterFunctionsCount = filterFunctions.size();

FloatFunction mixFilterFunctions(std::vector<fixed64> filters) {
	while (filters.size() < filterFunctionsCount) filters.push_back(0.f);
	return constrainFunction(normalizeFunction([=](fixed64 x) { 
		fixed64 sum = 0;
		for (int i=0;i<filterFunctionsCount;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctionsPtr->at(i)(x);
		return sum;
	}));
}

FloatFunction createInverseFunction(FloatFunction originalFunction, fixed64 epsilon) {
		const bool minus = originalFunction(0) > originalFunction(1);    
		return [=](fixed64 y) {
				fixed64 left = 0;
				fixed64 right = 1;
				if (minus) y = -y;
				while (right - left > epsilon) {
						const fixed64 mid = (left + right) / 2;
						fixed64 value = originalFunction(mid);
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
	fixed64 frac = 1.f / count;
	return [=](fixed64 x) {
		unsigned i = std::floor(x * count);
		fixed64 ifrac = i * frac;
		fixed64 xf = (x - ifrac) * count;
		fixed64 rp = (i & 1) ? 1.f - originalFunction(1.f - xf) : originalFunction(xf);
		return rp * frac + ifrac;
	};
}
