#pragma once

#include "hal/IGpioOutput.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockGpioOutput : public hal::IGpioOutput
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(bool, write, (bool level), (override));
    };
}
