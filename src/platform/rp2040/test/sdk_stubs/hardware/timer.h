#pragma once

#include <cstdint>
#include "PicoSdkStubState.hpp"

inline std::uint64_t time_us_64()
{
    return p3d::test::sdk_stub::state().currentTimestampUs;
}
