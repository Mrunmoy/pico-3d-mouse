#pragma once

#include "drivers/IAdcChannelReader.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockAdcChannelReader : public drivers::IAdcChannelReader
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(bool, read, (drivers::AdcSample& outSample), (override));
    };
}
