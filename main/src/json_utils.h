#pragma once

#include <ArduinoJson.h>
#include "common_types.h"

namespace ArduinoJson {
    template <>
    struct Converter<fixed64> {
        static void toJson(const fixed64& src, JsonVariant dst) {
            dst.set((float)src);
        }

        static fixed64 fromJson(JsonVariantConst src) {
            return (fixed64)src.as<float>();
        }

        static bool checkJson(JsonVariantConst src) {
            return src.is<float>() || src.is<int>();
        }
    };
}
