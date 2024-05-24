#pragma once

#include <vector>
#include <functional>
#include "fixedpoint.h"

using calc_type = FixedPoint<int32_t, int64_t, 20>;
using AnalogAutoReadFunction = std::function<calc_type(void)>;
using DigitalWriteFunction = std::function<void(int, bool)>;
using AutoDelayFunction = std::function<void(void)>;
using ResultCallback = std::function<void(int, calc_type)>;
using PinVector = std::vector<int>;
using SignalVector = std::vector<calc_type>;


calc_type** newMatrix(unsigned cols, unsigned rows);
void deleteMatrix(calc_type** matrix);
bool gauss(calc_type** matrix, unsigned cols, unsigned rows, calc_type** output=NULL, unsigned* rowOrder=NULL, bool* used=NULL);
void matMul(calc_type** a, calc_type** b, unsigned a_rows, unsigned common_size, unsigned b_cols, calc_type** output);

SignalVector gainSignal(
    const PinVector& colPins, const PinVector& rowPins,
    const AnalogAutoReadFunction readFunction,
    const DigitalWriteFunction writeFunction,
    const AutoDelayFunction delayFunction
);

calc_type calculateVoltage(calc_type* resistors, unsigned cols, unsigned rows, unsigned combination);
SignalVector calculateVoltage2(calc_type* resistors, unsigned cols, unsigned rows);
calc_type calculateDifference(SignalVector dest, calc_type* resistors, unsigned cols, unsigned rows);
SignalVector calculatePotentiometers(SignalVector dest, unsigned cols, unsigned rows);

void calculate(
    const PinVector& colPins, const PinVector& rowPins,
    const AnalogAutoReadFunction readFunction,
    const DigitalWriteFunction writeFunction,
    const AutoDelayFunction delayFunction,
    const ResultCallback resultCallback
);
