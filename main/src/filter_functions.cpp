#include <array>
#include "filter_functions.h"
#include "lib/fixedpoint/polyapprox.h"


MixedFunction::MixedFunction(ArithmeticFunction fun) {
	approximation.fit<fixed64_f>(fun, 0, 1);
	fmin = approximation(0);
	fmax = approximation(1);
	minff = std::min(fmin, fmax);
	absfdiff = std::abs(fmax-fmin);
}


MixedFunction::MixedFunction(ArithmeticFloatFunction fun) {
	approximation.fit<float>(fun, 0, 1);
	fmin = approximation(0);
	fmax = approximation(1);
	minff = std::min(fmin, fmax);
	absfdiff = std::abs(fmax-fmin);
}


fixed64_f MixedFunction::operator()(fixed64_f x) const {
	fixed64_f y = (approximation(x)-minff) / absfdiff;
	return (y < 0) ? fixed64_f(0) : (y > 1) ? fixed64_f(1) : y;
}


ArithmeticFloatFunction normalizeFunction(ArithmeticFloatFunction fun, fixed64_f min_x=0, fixed64_f max_x=1) {
	const fixed64_f fmin = fun(min_x);
	const fixed64_f fmax = fun(max_x);
	const fixed64_f x_diff = max_x-min_x;
	const fixed64_f minff = std::min(fmin, fmax);
	const fixed64_f absfdiff = std::abs(fmax-fmin);
	return [=](fixed64_f x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}


ArithmeticFloatFunction symFunction(ArithmeticFloatFunction fun) {
	return [=](fixed64_f x) { return 1-fun(1-x); };
}


constexpr const unsigned filterFunctionsCount = 9;
const std::array<ArithmeticFloatFunction, filterFunctionsCount> filterFunctions = {
	[](float x) {return x; },
	[](float x) {return x*x; },
	[](float x) {return sqrt(x); },
	normalizeFunction([](float x) { return exp(M_PI*(x-1)); }),
	normalizeFunction([](float x) { return asin(x*2-1); }),
	normalizeFunction([](float x) { return cos((x - 1) * M_PI); }),
	symFunction([](float x) {return x*x; }),
	symFunction([](float x) {return sqrt(x); }),
	normalizeFunction(symFunction([](float x) { return exp(M_PI*(x-1)); }))
};


MixedFunction mixFilterFunctions(const std::vector<float>& filters) {
	return MixedFunction((ArithmeticFloatFunction)([&](float x) {
		fixed64_f sum = 0;
		unsigned loopEnd = std::min(filterFunctionsCount, filters.size());
		for (int i=0;i<loopEnd;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctions[i](x);
		return sum;
	}));
}

fixed64_f calulcateInversedValue(const ArithmeticFunction& originalFunction, fixed64_f y, fixed64_f epsilon) {
	fixed64_f of_zero = originalFunction(0);
	fixed64_f of_one = originalFunction(1);
	const bool minus = of_zero > of_one;
	if (of_zero == y) return (fixed64_f)0;
	if (of_one == y) return (fixed64_f)1;
	fixed64_f left = 0;
	fixed64_f right = 1;
	fixed64_f prev_mid = -1;
	unsigned it = 0;
	if (minus) y = -y;
	while (right - left >= epsilon && it++ < 64) {
		const fixed64_f mid = (left + right) / 2;
		if (mid == prev_mid) break;
		prev_mid = mid;
		fixed64_f value = originalFunction(mid);
		if (minus) value = -value;
		if (value < y)
			left = mid;
		else
			right = mid;
	}
	return left;
}

