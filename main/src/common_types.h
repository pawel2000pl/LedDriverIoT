#pragma once

#include "fixedpoint.h"
#include <cstdint>
#include <array>

using fixed64 = fixedpoint<std::int64_t>;

using ColorChannels = std::array<fixed64, 4>;

