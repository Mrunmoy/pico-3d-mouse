#pragma once

#include "hal/ITimeProvider.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockTimeProvider : public hal::ITimeProvider
    {
    public:
        MOCK_METHOD(void, delayMs, (std::uint32_t ms), (override));
        MOCK_METHOD(void, delayUs, (std::uint32_t us), (override));
        MOCK_METHOD(std::uint64_t, getTimestampUs, (), (const, override));
    };
}
