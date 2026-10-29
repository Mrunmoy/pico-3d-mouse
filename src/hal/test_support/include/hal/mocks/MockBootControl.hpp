#pragma once

#include "hal/IBootControl.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockBootControl : public hal::IBootControl
    {
    public:
        MOCK_METHOD(void, enterBootloader, (), (override));
    };
}
