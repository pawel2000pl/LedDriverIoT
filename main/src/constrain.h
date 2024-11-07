#pragma once
#include <Arduino.h>

#undef constrain

template<typename T>
T constrain(T value, T lo, T hi) {
    return value <= lo ? lo : value >= hi ? hi : value;
}
