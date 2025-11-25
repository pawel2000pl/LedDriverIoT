#pragma once

#include "fixedpoint.h"
#include <cstdint>
#include <array>

using fixed32_c = fixedpoint<std::int32_t, std::int32_t, 16>;
using fixed32_f = fixedpoint<std::int32_t, std::int64_t, 21>;
using ColorChannels = std::array<fixed32_c, 4>;

