#include "domain/AppController.hpp"
#include "domain/CalibrationStore.hpp"
#include "drivers/mocks/MockPotentiometerSensor.hpp"
#include "hal/mocks/MockBootControl.hpp"
#include "hal/mocks/MockGpioOutput.hpp"
#include "hal/mocks/MockHidTransport.hpp"
#include "hal/mocks/MockNvmStorage.hpp"
#include "hal/mocks/MockSerialIo.hpp"
#include "hal/mocks/MockTimeProvider.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    using ::testing::_;
    using ::testing::DoAll;
    using ::testing::HasSubstr;
    using ::testing::InSequence;
    using ::testing::Return;
    using ::testing::ReturnRef;
    using ::testing::SetArgReferee;

    namespace
    {
        void expectSerialInput(MockSerialIo& serial, std::string_view text)
        {
            InSequence seq;
            for (const char c : text)
            {
                EXPECT_CALL(serial, readCharNonBlocking())
                    .WillOnce(Return(std::optional<char>{ c }));
            }
            EXPECT_CALL(serial, readCharNonBlocking()).WillOnce(Return(std::nullopt));
        }
    }

    TEST(AppControllerTest, StartFallsBackToLiveCenterCalibrationWhenFlashIsBlank)
    {
        MockPotentiometerSensor pots;
        MockNvmStorage          nvm;
        nvm.useBackingStore(256);
        domain::CalibrationStore calStore(nvm);
        MockHidTransport         hid;
        MockBootControl          boot;
        MockGpioOutput           led;
        MockSerialIo             serial;
        MockTimeProvider         time;

        domain::AppConfig cfg{};
        cfg.startupSettleMs   = 100;
        cfg.calibrationSweeps = 32;

        const std::array<std::uint16_t, 3> kCenters{ 2116, 2111, 2109 };
        ON_CALL(pots, centerCounts()).WillByDefault(ReturnRef(kCenters));

        domain::AppController app(pots, calStore, hid, boot, led, serial, time, cfg);

        {
            InSequence seq;
            EXPECT_CALL(serial, init()).WillOnce(Return(true));
            EXPECT_CALL(hid, init()).WillOnce(Return(true));
            EXPECT_CALL(led, init()).WillOnce(Return(true));
            EXPECT_CALL(led, write(true)).WillOnce(Return(true));
            EXPECT_CALL(pots, init()).WillOnce(Return(true));
            EXPECT_CALL(time, delayMs(100));
            EXPECT_CALL(pots, calibrateCenter(32)).WillOnce(Return(true));
            EXPECT_CALL(serial, writeLine(HasSubstr("@CAL CENTER_CALIBRATED")));
        }

        EXPECT_TRUE(app.start());
        EXPECT_EQ(app.calibration().centerCounts, kCenters);
    }

    TEST(AppControllerTest, StartLoadsSavedCalibrationFromFlashWithoutReZeroing)
    {
        MockPotentiometerSensor pots;
        MockNvmStorage          nvm;
        nvm.useBackingStore(256);
        domain::CalibrationStore calStore(nvm);

        domain::CalibrationData saved{};
        saved.centerCounts   = { 2100, 2050, 2080 };
        saved.minCounts      = { 100, 120, 140 };
        saved.maxCounts      = { 3900, 3950, 4000 };
        saved.deadzoneCounts = { 20, 25, 15 };
        saved.invert         = { true, false, true };
        ASSERT_TRUE(calStore.save(saved));

        MockHidTransport hid;
        MockBootControl  boot;
        MockGpioOutput   led;
        MockSerialIo     serial;
        MockTimeProvider time;

        domain::AppController app(pots, calStore, hid, boot, led, serial, time);

        EXPECT_CALL(serial, init()).WillOnce(Return(true));
        EXPECT_CALL(hid, init()).WillOnce(Return(true));
        EXPECT_CALL(led, init()).WillOnce(Return(true));
        EXPECT_CALL(led, write(true)).WillOnce(Return(true));
        EXPECT_CALL(pots, init()).WillOnce(Return(true));
        EXPECT_CALL(time, delayMs(_));
        EXPECT_CALL(pots, calibrateCenter(_)).Times(0);
        EXPECT_CALL(serial, writeLine(HasSubstr("@CAL LOADED")));

        ASSERT_TRUE(app.start());
        EXPECT_EQ(app.calibration().centerCounts, saved.centerCounts);
        EXPECT_EQ(app.calibration().deadzoneCounts, saved.deadzoneCounts);
        EXPECT_EQ(app.calibration().invert, saved.invert);
    }

    TEST(AppControllerTest, TickProcessesAxesSends6DofHidReportAndTogglesLed)
    {
        MockPotentiometerSensor pots;
        MockNvmStorage          nvm;
        nvm.useBackingStore(256);
        domain::CalibrationStore calStore(nvm);
        MockHidTransport         hid;
        MockBootControl          boot;
        MockGpioOutput           led;
        MockSerialIo             serial;
        MockTimeProvider         time;

        domain::AppConfig cfg{};
        cfg.samplePeriodMs         = 20;
        cfg.ledToggleIntervalTicks = 2;
        for (auto& ax : cfg.axisConfigs)
        {
            ax.deadzoneCounts      = 10;
            ax.maxDeflectionCounts = 1000;
            ax.outputScaleMax      = 500;
        }

        domain::AppController app(pots, calStore, hid, boot, led, serial, time, cfg);

        drivers::PotentiometerSnapshot snap{};
        snap.rawCounts  = { 2558, 1538, 2258 };
        snap.millivolts = { 2061, 1239, 1819 };

        // Tick 1
        EXPECT_CALL(hid, task());
        EXPECT_CALL(serial, readCharNonBlocking()).WillOnce(Return(std::nullopt));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(snap), Return(true)));
        EXPECT_CALL(hid, sendMotionReport(_)).WillOnce(Return(true));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)[raw=2558")));
        EXPECT_CALL(led, write(_)).Times(0);
        EXPECT_CALL(time, delayMs(20));

        ASSERT_TRUE(app.tick());
        EXPECT_EQ(app.lastFrame().hidReport.tx, -250);
        EXPECT_EQ(app.lastFrame().hidReport.ty, 250);
        EXPECT_EQ(app.lastFrame().hidReport.tz, 100);
        EXPECT_EQ(app.lastFrame().hidReport.rx, 250);
        EXPECT_EQ(app.lastFrame().hidReport.ry, -250);
        EXPECT_EQ(app.lastFrame().hidReport.rz, 100);

        // Tick 2: toggles status LED
        EXPECT_CALL(hid, task());
        EXPECT_CALL(serial, readCharNonBlocking()).WillOnce(Return(std::nullopt));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(snap), Return(true)));
        EXPECT_CALL(hid, sendMotionReport(_)).WillOnce(Return(true));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)[raw=2558")));
        EXPECT_CALL(led, write(false)).WillOnce(Return(true));
        EXPECT_CALL(time, delayMs(20));

        ASSERT_TRUE(app.tick());
    }

    TEST(AppControllerTest, MinMaxCaptureAndSaveCalibrationWorkflow)
    {
        MockPotentiometerSensor pots;
        MockNvmStorage          nvm;
        nvm.useBackingStore(256);
        domain::CalibrationStore calStore(nvm);
        MockHidTransport         hid;
        MockBootControl          boot;
        MockGpioOutput           led;
        MockSerialIo             serial;
        MockTimeProvider         time;

        domain::AppController app(pots, calStore, hid, boot, led, serial, time);

        // Tick 1: Start Min/Max capture and observe extreme low reading
        expectSerialInput(serial, "CAL_MINMAX_START\n");
        drivers::PotentiometerSnapshot lowSnap{};
        lowSnap.rawCounts  = { 120, 150, 180 };
        lowSnap.millivolts = { 96, 120, 145 };

        EXPECT_CALL(hid, task());
        EXPECT_CALL(serial, writeLine(HasSubstr("@CAL MINMAX_CAPTURE_STARTED")));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(lowSnap), Return(true)));
        EXPECT_CALL(hid, sendMotionReport(_)).WillOnce(Return(true));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)")));
        EXPECT_CALL(time, delayMs(_));
        ASSERT_TRUE(app.tick());
        EXPECT_TRUE(app.isCapturingMinMax());

        // Tick 2: Observe extreme high reading, end Min/Max capture, and save to Flash
        expectSerialInput(serial, "CAL_MINMAX_END\nSAVE_CAL\n");
        drivers::PotentiometerSnapshot highSnap{};
        highSnap.rawCounts  = { 3980, 3920, 4010 };
        highSnap.millivolts = { 3207, 3158, 3231 };

        // Before ending, feed one high sample during Tick 2 or end after:
        // Wait: commands run at start of tick(), so let's do high reading first, then end + save on Tick 3!
        EXPECT_CALL(hid, task());
        EXPECT_CALL(serial, writeLine(HasSubstr("@CAL MINMAX_CAPTURED")));
        EXPECT_CALL(serial, writeLine(HasSubstr("@CAL SAVED_OK")));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(highSnap), Return(true)));
        EXPECT_CALL(hid, sendMotionReport(_)).WillOnce(Return(true));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)")));
        EXPECT_CALL(time, delayMs(_));
        ASSERT_TRUE(app.tick());
        EXPECT_FALSE(app.isCapturingMinMax());
        EXPECT_EQ(app.calibration().minCounts[0], 120U);

        // Verify saved to NVM
        domain::CalibrationData persisted{};
        ASSERT_TRUE(calStore.load(persisted));
        EXPECT_EQ(persisted.minCounts[0], 120U);
    }

    TEST(AppControllerTest, BootselCommandTriggersButtonlessBootloaderEntry)
    {
        MockPotentiometerSensor pots;
        MockNvmStorage          nvm;
        nvm.useBackingStore(256);
        domain::CalibrationStore calStore(nvm);
        MockHidTransport         hid;
        MockBootControl          boot;
        MockGpioOutput           led;
        MockSerialIo             serial;
        MockTimeProvider         time;

        domain::AppController app(pots, calStore, hid, boot, led, serial, time);

        expectSerialInput(serial, "BOOTSEL\n");

        drivers::PotentiometerSnapshot snap{};
        snap.rawCounts  = { 2048, 2048, 2048 };
        snap.millivolts = { 1650, 1650, 1650 };

        EXPECT_CALL(hid, task());
        EXPECT_CALL(serial, writeLine(HasSubstr("@BOOT ENTERING_BOOTSEL")));
        EXPECT_CALL(boot, enterBootloader());
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(snap), Return(true)));
        EXPECT_CALL(hid, sendMotionReport(_)).WillOnce(Return(true));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)")));
        EXPECT_CALL(time, delayMs(_));

        EXPECT_TRUE(app.tick());
    }
}
