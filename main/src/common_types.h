#pragma once

#include "fixedpoint.h"
#include <cstdint>
#include <array>

using fraction64 = fixedpoint<std::int64_t, std::int_fast64_t, 40>;
using fraction32 = fixedpoint<std::int32_t, std::int_fast32_t, 20>;

using ColorChannels = std::array<fraction32, 4>;

