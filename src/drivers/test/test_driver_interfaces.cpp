#include "drivers/mocks/MockAdcChannelReader.hpp"
#include "drivers/mocks/MockPotentiometerSensor.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    TEST(DriverInterfacesTest, AxisIdMappingMatchesPhysicalWiring)
    {
        EXPECT_EQ(static_cast<std::size_t>(drivers::AxisId::ForwardBackward), 0U);
        EXPECT_EQ(static_cast<std::size_t>(drivers::AxisId::LeftRight), 1U);
        EXPECT_EQ(static_cast<std::size_t>(drivers::AxisId::Rotation), 2U);
        EXPECT_EQ(drivers::kNumPotAxes, 3U);
    }
}
