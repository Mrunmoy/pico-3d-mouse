#pragma once

#include "hal/IAdcHardware.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockAdcHardware : public hal::IAdcHardware
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(bool, initChannel, (std::uint8_t channel), (override));
        MOCK_METHOD(bool, selectChannel, (std::uint8_t channel), (override));
        MOCK_METHOD(bool, readRaw, (std::uint16_t& outCounts), (override));
    };
}
