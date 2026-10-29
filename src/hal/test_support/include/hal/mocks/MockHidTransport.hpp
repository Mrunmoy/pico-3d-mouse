#pragma once

#include "hal/IHidTransport.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockHidTransport : public hal::IHidTransport
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(void, task, (), (override));
        MOCK_METHOD(bool, sendMotionReport, (const hal::MotionReport6Dof& report), (override));
    };
}
