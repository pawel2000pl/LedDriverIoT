#include "filter_functions.h"
#include "taylormath.h"

ArithmeticFunction normalizeFunction(ArithmeticFunction fun, fixed64 min_x, fixed64 max_x) {
	const fixed64 fmin = fun(min_x);
	const fixed64 fmax = fun(max_x);
	const fixed64 x_diff = max_x-min_x;
	const fixed64 minff = std::min(fmin, fmax);
	const fixed64 absfdiff = std::abs(fmax-fmin);
	return [=](fixed64 x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}

ArithmeticFunction constrainFunction(ArithmeticFunction fun, fixed64 min_y, fixed64 max_y) {
	return [=](fixed64 x) {
		fixed64 y = fun(x);
		return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
	};
}

ArithmeticFunction symFunction(ArithmeticFunction fun) {
	return [=](fixed64 x) { return 1-fun(1-x); };
}


const std::vector<ArithmeticFunction> filterFunctions = {
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
const std::vector<ArithmeticFunction>* filterFunctionsPtr = &filterFunctions;
const unsigned filterFunctionsCount = filterFunctions.size();

ArithmeticFunction mixFilterFunctions(std::vector<fixed64> filters) {
	while (filters.size() < filterFunctionsCount) filters.push_back(0);
	return constrainFunction(normalizeFunction([=](fixed64 x) { 
		fixed64 sum = 0;
		for (int i=0;i<filterFunctionsCount;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctionsPtr->at(i)(x);
		return sum;
	}));
}

ArithmeticFunction createInverseFunction(ArithmeticFunction originalFunction, fixed64 epsilon) {
	fixed64 of_zero = originalFunction(0);
	fixed64 of_one = originalFunction(1);
	const bool minus = of_zero > of_one;    
	return [=](fixed64 y) {
		if (of_zero == y) return (fixed64)0;
		if (of_one == y) return (fixed64)1;
		fixed64 left = 0;
		fixed64 right = 1;
		fixed64 prev_mid = -1;
		if (minus) y = -y;
		while (right - left >= epsilon) {
			const fixed64 mid = (left + right) / 2;
			if (mid == prev_mid) break;
			prev_mid = mid;
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

ArithmeticFunction periodizeFunction(ArithmeticFunction originalFunction, unsigned count) {
	fixed64 frac = fixed64(1) / count;
	return [=](fixed64 x) {
		unsigned i = std::floor(x * count);
		fixed64 ifrac = i * frac;
		fixed64 xf = (x - ifrac) * count;
		fixed64 rp = (i & 1) ? fixed64(1) - originalFunction(fixed64(1) - xf) : originalFunction(xf);
		return rp * frac + ifrac;
	};
}
