#include <array>
#include "filter_functions.h"
#include "lib/fixedpoint/taylormath.h"
#include "lib/fixedpoint/polyapprox.h"


MixedFunction::MixedFunction(ArithmeticFunction fun) {
	approximation.fit<fixed32_f>(fun, 0, 1);
	fmin = approximation(0);
	fmax = approximation(1);
	minff = std::min(fmin, fmax);
	absfdiff = std::abs(fmax-fmin);
}


fixed32_f MixedFunction::operator()(fixed32_f x) const {
	fixed32_f y = (approximation(x)-minff) / absfdiff;
	return (y < 0) ? fixed32_f(0) : (y > 1) ? fixed32_f(0) : y;
}


ArithmeticFunction normalizeFunction(ArithmeticFunction fun, fixed32_f min_x, fixed32_f max_x) {
	const fixed32_f fmin = fun(min_x);
	const fixed32_f fmax = fun(max_x);
	const fixed32_f x_diff = max_x-min_x;
	const fixed32_f minff = std::min(fmin, fmax);
	const fixed32_f absfdiff = std::abs(fmax-fmin);
	return [=](fixed32_f x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}


ArithmeticFunction constrainFunction(ArithmeticFunction fun, fixed32_f min_y, fixed32_f max_y) {
	return [=](fixed32_f x) {
		fixed32_f y = fun(x);
		return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
	};
}


ArithmeticFunction symFunction(ArithmeticFunction fun) {
	return [=](fixed32_f x) { return 1-fun(1-x); };
}


constexpr const unsigned filterFunctionsCount = 9;
const std::array<ArithmeticFunction, filterFunctionsCount> filterFunctions = {
	[](fixed32_f x) {return x; },
	[](fixed32_f x) {return x*x; },
	[](fixed32_f x) {return taylor::sqrt<fixed32_f>(x); },
	normalizeFunction([](fixed32_f x) { return taylor::exp<fixed32_f>(M_PI*(x-1)); }),
	normalizeFunction([](fixed32_f x) { return taylor::asin<fixed32_f>(x*2-1); }),
	normalizeFunction([](fixed32_f x) { return taylor::cos<fixed32_f>((x - 1) * M_PI); }),
	symFunction([](fixed32_f x) {return x*x; }),
	symFunction([](fixed32_f x) {return taylor::sqrt<fixed32_f>(x); }),
	normalizeFunction(symFunction([](fixed32_f x) { return taylor::exp<fixed32_f>(M_PI*(x-1)); }))
};


MixedFunction mixFilterFunctions(const std::vector<fixed32_f>& filters) {
	return MixedFunction([&](fixed32_f x) { 
		fixed32_f sum = 0;
		unsigned loopEnd = std::min(filterFunctionsCount, filters.size());
		for (int i=0;i<loopEnd;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctions[i](x);
		return sum;
	});
}

fixed32_f calulcateInversedValue(const ArithmeticFunction& originalFunction, fixed32_f y, fixed32_f epsilon) {
	fixed32_f of_zero = originalFunction(0);
	fixed32_f of_one = originalFunction(1);
	const bool minus = of_zero > of_one;  
	if (of_zero == y) return (fixed32_f)0;
	if (of_one == y) return (fixed32_f)1;
	fixed32_f left = 0;
	fixed32_f right = 1;
	fixed32_f prev_mid = -1;
	unsigned it = 0;
	if (minus) y = -y;
	while (right - left >= epsilon && it++ < 64) {
		const fixed32_f mid = (left + right) / 2;
		if (mid == prev_mid) break;
		prev_mid = mid;
		fixed32_f value = originalFunction(mid);
		if (minus) value = -value;
		if (value < y)
			left = mid;
		else
			right = mid;
	}
	return left; 
}

