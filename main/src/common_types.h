#pragma once

#include "fixedpoint.h"
#include <cstdint>
#include <array>

using fixed64 = fixedpoint<std::int64_t, std::int64_t, 31>;

using ColorChannels = std::array<fixed64, 4>;

