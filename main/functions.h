#pragma once

#include <math.h>
#include <functional>
#include "json.h"

using FloatFunction = std::function<float(float)>;
extern FloatFunction filterFunctions[];

FloatFunction normalizeFunction(FloatFunction fun, float min_x=0, float max_x=0);
FloatFunction constrainFunction(FloatFunction fun, float min_y=0, float max_y=1);
FloatFunction symFunction(FloatFunction fun);
FloatFunction mixFilterFunctions(const JsonArrayConst& filters);
