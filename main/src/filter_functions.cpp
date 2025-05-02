#include "filter_functions.h"
#include "taylormath.h"

FloatFunction normalizeFunction(FloatFunction fun, fraction32 min_x, fraction32 max_x) {
	const fraction32 fmin = fun(min_x);
	const fraction32 fmax = fun(max_x);
	const fraction32 x_diff = max_x-min_x;
	const fraction32 minff = min(fmin, fmax);
	const fraction32 absfdiff = abs(fmax-fmin);
	return [=](fraction32 x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}

FloatFunction constrainFunction(FloatFunction fun, fraction32 min_y, fraction32 max_y) {
	return [=](fraction32 x) {
		fraction32 y = fun(x);
		return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
	};
}

FloatFunction symFunction(FloatFunction fun) {
	return [=](fraction32 x) { return 1-fun(1-x); };
}


const std::vector<FloatFunction> filterFunctions = {
	[](fraction32 x) {return x; },
	[](fraction32 x) {return x*x; },
	[](fraction32 x) {return taylor::sqrt<fraction32>(x); },
	normalizeFunction([](fraction32 x) { return taylor::exp<fraction32>(M_PI*(x-1)); }),
	normalizeFunction([](fraction32 x) { return taylor::asin<fraction32>(x*2-1); }),
	normalizeFunction([](fraction32 x) { return taylor::cos<fraction32>((x - 1) * M_PI); }),
	symFunction([](fraction32 x) {return x*x; }),
	symFunction([](fraction32 x) {return taylor::sqrt<fraction32>(x); }),
	normalizeFunction(symFunction([](fraction32 x) { return taylor::exp<fraction32>(M_PI*(x-1)); }))
};
const std::vector<FloatFunction>* filterFunctionsPtr = &filterFunctions;
const unsigned filterFunctionsCount = filterFunctions.size();

FloatFunction mixFilterFunctions(std::vector<fraction32> filters) {
	while (filters.size() < filterFunctionsCount) filters.push_back(0);
	return constrainFunction(normalizeFunction([=](fraction32 x) { 
		fraction32 sum = 0;
		for (int i=0;i<filterFunctionsCount;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctionsPtr->at(i)(x);
		return sum;
	}));
}

FloatFunction createInverseFunction(FloatFunction originalFunction, fraction32 epsilon) {
		const bool minus = originalFunction(0) > originalFunction(1);    
		return [=](fraction32 y) {
				fraction32 left = 0;
				fraction32 right = 1;
				fraction32 prev_mid = -1;
				if (minus) y = -y;
				while (right - left > epsilon) {
						const fraction32 mid = (left + right) / 2;
						if (mid == prev_mid) break;
						prev_mid = mid;
						fraction32 value = originalFunction(mid);
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
	fraction32 frac = fraction32(1) / count;
	return [=](fraction32 x) {
		unsigned i = std::floor(x * count);
		fraction32 ifrac = i * frac;
		fraction32 xf = (x - ifrac) * count;
		fraction32 rp = (i & 1) ? fraction32(1) - originalFunction(fraction32(1) - xf) : originalFunction(xf);
		return rp * frac + ifrac;
	};
}
