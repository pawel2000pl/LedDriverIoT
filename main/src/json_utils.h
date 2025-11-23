#pragma once

#include "lib/ArduinoJson/ArduinoJson.h"
#include "common_types.h"

namespace ArduinoJson {
    template<typename T, typename TC, unsigned frac_bits>
    struct Converter<fixedpoint<T, TC, frac_bits>> {

        using FP = fixedpoint<T, TC, frac_bits>;

        static void toJson(const FP& src, JsonVariant dst) {
            dst.set((float)src);
        }

        static FP fromJson(JsonVariantConst src) {
            return FP::from_float(src.as<float>());
        }

        static bool checkJson(JsonVariantConst src) {
            return src.is<float>() || src.is<int>();
        }
    };
}
