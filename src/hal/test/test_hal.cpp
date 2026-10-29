#include "hal/mocks/MockAdcHardware.hpp"
#include "hal/mocks/MockGpioOutput.hpp"
#include "hal/mocks/MockSerialIo.hpp"
#include "hal/mocks/MockTimeProvider.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    using ::testing::DoAll;
    using ::testing::Return;
    using ::testing::SetArgReferee;

    TEST(HalContractsTest, AdcHardwarePolymorphicDispatch)
    {
        MockAdcHardware mockAdc;
        hal::IAdcHardware& adc = mockAdc;

        EXPECT_CALL(mockAdc, init()).WillOnce(Return(true));
        EXPECT_CALL(mockAdc, initChannel(0)).WillOnce(Return(true));
        EXPECT_CALL(mockAdc, selectChannel(0)).WillOnce(Return(true));
        EXPECT_CALL(mockAdc, readRaw(::testing::_))
            .WillOnce(DoAll(SetArgReferee<0>(2048U), Return(true)));

        EXPECT_TRUE(adc.init());
        EXPECT_TRUE(adc.initChannel(0));
        EXPECT_TRUE(adc.selectChannel(0));
        std::uint16_t raw = 0;
        EXPECT_TRUE(adc.readRaw(raw));
        EXPECT_EQ(raw, 2048U);
    }

    TEST(HalContractsTest, GpioSerialAndTimePolymorphicDispatch)
    {
        MockGpioOutput   mockGpio;
        MockSerialIo     mockSerial;
        MockTimeProvider mockTime;

        hal::IGpioOutput&   gpio   = mockGpio;
        hal::ISerialIo&     serial = mockSerial;
        hal::ITimeProvider& time   = mockTime;

        EXPECT_CALL(mockGpio, init()).WillOnce(Return(true));
        EXPECT_CALL(mockGpio, write(true)).WillOnce(Return(true));
        EXPECT_CALL(mockSerial, init()).WillOnce(Return(true));
        EXPECT_CALL(mockSerial, readCharNonBlocking()).WillOnce(Return('z'));
        EXPECT_CALL(mockTime, getTimestampUs()).WillOnce(Return(123456ULL));

        EXPECT_TRUE(gpio.init());
        EXPECT_TRUE(gpio.write(true));
        EXPECT_TRUE(serial.init());
        EXPECT_EQ(serial.readCharNonBlocking(), 'z');
        EXPECT_EQ(time.getTimestampUs(), 123456ULL);
    }
}
