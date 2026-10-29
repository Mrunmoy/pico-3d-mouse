#include "platform/rp2040/BoardPins.hpp"
#include "platform/rp2040/Rp2040AdcHardware.hpp"
#include "platform/rp2040/Rp2040BootControl.hpp"
#include "platform/rp2040/Rp2040FlashStorage.hpp"
#include "platform/rp2040/Rp2040GpioOutput.hpp"
#include "platform/rp2040/Rp2040HidTransport.hpp"
#include "platform/rp2040/Rp2040TimeProvider.hpp"
#include "platform/rp2040/Rp2040UsbSerialIo.hpp"

#include "PicoSdkStubState.hpp"
#include "pico/stdlib.h"

#include <array>
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

        EXPECT_TRUE(adc.initChannel(0));
        EXPECT_TRUE(adc.initChannel(1));
        EXPECT_TRUE(adc.initChannel(2));
        EXPECT_FALSE(adc.initChannel(3));

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
        EXPECT_FALSE(serial.readCharNonBlocking().has_value());

        sdk_stub::state().nextGetcharResult = 'z';
        const auto ch = serial.readCharNonBlocking();
        ASSERT_TRUE(ch.has_value());
        EXPECT_EQ(*ch, 'z');
    }

    TEST_F(Rp2040PlatformTest, FlashStorageErasesSectorProgramsPageAndReadsBack)
    {
        rp::Rp2040FlashStorage flash{};

        std::array<std::uint8_t, 8> payload{ 1, 2, 3, 4, 5, 6, 7, 8 };
        ASSERT_TRUE(flash.write(payload.data(), payload.size()));
        EXPECT_EQ(sdk_stub::state().flashEraseCount, 1U);
        EXPECT_EQ(sdk_stub::state().flashProgramCount, 1U);

        std::array<std::uint8_t, 8> readBack{};
        ASSERT_TRUE(flash.read(readBack.data(), readBack.size()));
        EXPECT_EQ(readBack, payload);

        EXPECT_FALSE(flash.write(nullptr, 8));
        EXPECT_FALSE(flash.read(nullptr, 8));
        EXPECT_FALSE(flash.write(payload.data(), 1024));
    }

    TEST_F(Rp2040PlatformTest, BootControlCallsResetUsbBoot)
    {
        rp::Rp2040BootControl boot{};
        EXPECT_FALSE(sdk_stub::state().bootloaderEntered);
        boot.enterBootloader();
        EXPECT_TRUE(sdk_stub::state().bootloaderEntered);
    }

    TEST_F(Rp2040PlatformTest, HidTransportStreamsWhenDeflectedAndSuppressesIdleZeroPackets)
    {
        rp::Rp2040HidTransport hid{};
        EXPECT_TRUE(hid.init());

        hal::MotionReport6Dof r{};
        r.tx = 120;
        r.ty = -80;

        sdk_stub::state().hidReady = false;
        EXPECT_FALSE(hid.sendMotionReport(r));

        sdk_stub::state().hidReady = true;
        EXPECT_TRUE(hid.sendMotionReport(r));
        EXPECT_EQ(sdk_stub::state().hidReportCount, 1U);
        EXPECT_EQ(sdk_stub::state().lastHidReportId, rp::Rp2040HidTransport::kReportIdTranslation);

        // Holding non-zero deflection continues streaming relative reports for 3DxWare / Fusion 360
        EXPECT_TRUE(hid.sendMotionReport(r));
        EXPECT_EQ(sdk_stub::state().hidReportCount, 2U);
        EXPECT_EQ(sdk_stub::state().lastHidReportId, rp::Rp2040HidTransport::kReportIdTranslation);

        // Simultaneous translation + rotation alternates between Report ID 1 and Report ID 2
        r.rz = 200;
        EXPECT_TRUE(hid.sendMotionReport(r));
        EXPECT_EQ(sdk_stub::state().hidReportCount, 3U);
        EXPECT_EQ(sdk_stub::state().lastHidReportId, rp::Rp2040HidTransport::kReportIdRotation);

        // Returning to center (0,0,0) sends final zero reports and then stays silent at rest
        r = hal::MotionReport6Dof{};
        EXPECT_TRUE(hid.sendMotionReport(r)); // zeros translation
        EXPECT_EQ(sdk_stub::state().hidReportCount, 4U);
        EXPECT_TRUE(hid.sendMotionReport(r)); // zeros rotation
        EXPECT_EQ(sdk_stub::state().hidReportCount, 5U);
        EXPECT_TRUE(hid.sendMotionReport(r)); // idle at rest -> no packet sent
        EXPECT_EQ(sdk_stub::state().hidReportCount, 5U);
    }
}
