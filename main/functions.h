#include <math.h>
#include <functional>
#include "json.h"

using FloatFunction = std::function<float(float)>;

FloatFunction normalizeFunction(FloatFunction fun, float min_x=0, float max_x=0) {
  const float fmin = fun(min_x);
  const float fmax = fun(max_x);
  const float x_diff = max_x-min_x;
  const float minff = min(fmin, fmax);
  const float absfdiff = abs(fmax-fmin);
  return [fun, min_x, x_diff, minff, absfdiff](float x) { return (fun(x*x_diff+min_x)-minff) / absfdiff; };
}

FloatFunction constrainFunction(FloatFunction fun, float min_y=0, float max_y=1) {
  return [fun, min_y, max_y](float x) {
    float y = fun(x);
    return (y < min_y) ? min_y : (y > max_y) ? max_y : y;
  };
}

FloatFunction symFunction(FloatFunction fun) {
  return [fun](float x) { return 1-fun(1-x); };
}

FloatFunction filterFunctions[] = {
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

FloatFunction mixFilterFunctions(const JsonArrayConst& filters) {
  float values[9];
  for (int i=0;i<9;i++)
    values[i] = filters[i].as<float>();
  return constrainFunction(normalizeFunction([values, filterFunctions](float x) { 
    float sum = 0;
    for (int i=0;i<9;i++)
      sum += values[i] * filterFunctions[i](x);
    return sum;
  }));
}

