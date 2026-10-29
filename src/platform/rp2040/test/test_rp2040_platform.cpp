#include "platform/rp2040/BoardPins.hpp"
#include "platform/rp2040/Rp2040AdcHardware.hpp"
#include "platform/rp2040/Rp2040GpioOutput.hpp"
#include "platform/rp2040/Rp2040TimeProvider.hpp"
#include "platform/rp2040/Rp2040UsbSerialIo.hpp"

#include "PicoSdkStubState.hpp"
#include "pico/stdlib.h"

#include <gtest/gtest.h>

namespace p3d::test
{
    namespace rp = p3d::platform::rp2040;

    class Rp2040PlatformTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            sdk_stub::reset();
        }
    };

    TEST_F(Rp2040PlatformTest, AdcHardwareInitializesPinsAndRejectsOutOfRangeChannels)
    {
        rp::Rp2040AdcHardware adc{};

        EXPECT_TRUE(adc.initChannel(0));  // GP26 (Forward/Backward)
        EXPECT_TRUE(adc.initChannel(1));  // GP27 (Left/Right)
        EXPECT_TRUE(adc.initChannel(2));  // GP28 (Rotation)
        EXPECT_FALSE(adc.initChannel(3)); // Out of range

        EXPECT_TRUE(sdk_stub::state().adcInitialized);
        ASSERT_EQ(sdk_stub::state().adcInitializedGpios.size(), 3U);
        EXPECT_EQ(sdk_stub::state().adcInitializedGpios[0], 26U);
        EXPECT_EQ(sdk_stub::state().adcInitializedGpios[1], 27U);
        EXPECT_EQ(sdk_stub::state().adcInitializedGpios[2], 28U);
        EXPECT_EQ(sdk_stub::state().pullsDisabledGpios.size(), 3U);

        EXPECT_TRUE(adc.selectChannel(2));
        EXPECT_EQ(sdk_stub::state().selectedAdcInput, 2U);
        EXPECT_FALSE(adc.selectChannel(3));

        sdk_stub::state().nextAdcReadValue = 3124U;
        std::uint16_t raw = 0;
        EXPECT_TRUE(adc.readRaw(raw));
        EXPECT_EQ(raw, 3124U);
    }

    TEST_F(Rp2040PlatformTest, GpioOutputConfiguresDirectionAndDrivesLevel)
    {
        rp::Rp2040GpioOutput led{ rp::pins::kStatusLedGpio };

        EXPECT_TRUE(led.init());
        EXPECT_EQ(sdk_stub::state().lastGpioInitPin, 25U);
        EXPECT_TRUE(sdk_stub::state().lastGpioDirOut);

        EXPECT_TRUE(led.write(true));
        EXPECT_EQ(sdk_stub::state().lastGpioPutPin, 25U);
        EXPECT_TRUE(sdk_stub::state().lastGpioPutLevel);

        EXPECT_TRUE(led.write(false));
        EXPECT_FALSE(sdk_stub::state().lastGpioPutLevel);
    }

    TEST_F(Rp2040PlatformTest, TimeProviderForwardsDelaysAndReads64BitTimer)
    {
        rp::Rp2040TimeProvider time{};

        time.delayMs(50);
        EXPECT_EQ(sdk_stub::state().lastSleepMs, 50U);

        time.delayUs(250);
        EXPECT_EQ(sdk_stub::state().lastSleepUs, 250U);

        sdk_stub::state().currentTimestampUs = 987654321ULL;
        EXPECT_EQ(time.getTimestampUs(), 987654321ULL);
    }

    TEST_F(Rp2040PlatformTest, UsbSerialIoHandlesTimeoutAndCharacterInput)
    {
        rp::Rp2040UsbSerialIo serial{};

        sdk_stub::state().stdioInitResult = true;
        EXPECT_TRUE(serial.init());

        sdk_stub::state().nextGetcharResult = PICO_ERROR_TIMEOUT;
        EXPECT_EQ(serial.readCharNonBlocking(), -1);

        sdk_stub::state().nextGetcharResult = 'z';
        EXPECT_EQ(serial.readCharNonBlocking(), 'z');
    }
}
