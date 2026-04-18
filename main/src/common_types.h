#pragma once

#include <cstdint>
#include <array>
#include "lib/fixedpoint/fixedpoint.h"

using fixed32_c = fixedpoint<std::int32_t, std::int64_t, 16>;
using fixed64_f = fixedpoint<std::int64_t, std::int64_t, 24>;
using ColorChannels = std::array<fixed32_c, 4>;

