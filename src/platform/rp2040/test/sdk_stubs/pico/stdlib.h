#pragma once

#include <cstdint>
#include "PicoSdkStubState.hpp"

inline constexpr int PICO_ERROR_TIMEOUT = -1;

inline bool stdio_init_all()
{
    return p3d::test::sdk_stub::state().stdioInitResult;
}

inline int getchar_timeout_us(std::uint32_t /*timeout_us*/)
{
    return p3d::test::sdk_stub::state().nextGetcharResult;
}

inline void sleep_ms(std::uint32_t ms)
{
    p3d::test::sdk_stub::state().lastSleepMs = ms;
}

inline void sleep_us(std::uint64_t us)
{
    p3d::test::sdk_stub::state().lastSleepUs = static_cast<std::uint32_t>(us);
}
