#include "filter_functions.h"

FloatFunction normalizeFunction(FloatFunction fun, float min_x, float max_x) {
	const float fmin = fun(min_x);
	const float fmax = fun(max_x);
	const float x_diff = max_x-min_x;
	const float minff = min(fmin, fmax);
	const float absfdiff = abs(fmax-fmin);
	return [=](float x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}

FloatFunction constrainFunction(FloatFunction fun, float min_y, float max_y) {
	return [=](float x) {
		float y = fun(x);
		return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
	};
}

FloatFunction symFunction(FloatFunction fun) {
	return [=](float x) { return 1-fun(1-x); };
}

const std::vector<FloatFunction> filterFunctions = {
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
const std::vector<FloatFunction>* filterFunctionsPtr = &filterFunctions;
const unsigned filterFunctionsCount = filterFunctions.size();

FloatFunction mixFilterFunctions(std::vector<float> filters) {
	while (filters.size() < filterFunctionsCount) filters.push_back(0.f);
	return constrainFunction(normalizeFunction([=](float x) { 
		float sum = 0;
		for (int i=0;i<filterFunctionsCount;i++)
			if (filters[i] != 0)
				sum += filters[i] * filterFunctionsPtr->at(i)(x);
		return sum;
	}));
}

FloatFunction createInverseFunction(FloatFunction originalFunction, float epsilon) {
		const bool minus = originalFunction(0) > originalFunction(1);    
		return [=](float y) {
				float left = 0;
				float right = 1;
				if (minus) y = -y;
				while (right - left > epsilon) {
						const float mid = (left + right) / 2;
						float value = originalFunction(mid);
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
	float frac = 1.f / count;
	return [=](float x) {
		unsigned i = floor(x * count);
		float ifrac = i * frac;
		float xf = (x - ifrac) * count;
		float rp = (i & 1) ? 1.f - originalFunction(1.f - xf) : originalFunction(xf);
		return rp * frac + ifrac;
	};
}
