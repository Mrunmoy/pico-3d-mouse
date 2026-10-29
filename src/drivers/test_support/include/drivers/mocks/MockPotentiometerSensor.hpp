#pragma once

#include "drivers/IPotentiometerSensor.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockPotentiometerSensor : public drivers::IPotentiometerSensor
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(bool, calibrateCenter, (std::uint16_t numSweeps), (override));
        MOCK_METHOD(bool, readSnapshot, (drivers::PotentiometerSnapshot& outSnapshot), (override));
        MOCK_METHOD((const std::array<std::uint16_t, drivers::kNumPotAxes>&),
                    centerCounts,
                    (),
                    (const, override));
    };
}
