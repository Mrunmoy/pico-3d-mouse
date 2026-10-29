#include "domain/AppController.hpp"
#include "drivers/mocks/MockPotentiometerSensor.hpp"
#include "hal/mocks/MockGpioOutput.hpp"
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

    TEST(AppControllerTest, StartInitializesPeripheralsSettlesAndCalibratesCenter)
    {
        MockPotentiometerSensor pots;
        MockGpioOutput          led;
        MockSerialIo            serial;
        MockTimeProvider        time;

        domain::AppConfig cfg{};
        cfg.startupSettleMs   = 100;
        cfg.calibrationSweeps = 32;

        const std::array<std::uint16_t, 3> kCenters{ 2040, 2050, 2060 };
        ON_CALL(pots, centerCounts()).WillByDefault(ReturnRef(kCenters));

        domain::AppController app(pots, led, serial, time, cfg);

        {
            InSequence seq;
            EXPECT_CALL(serial, init()).WillOnce(Return(true));
            EXPECT_CALL(led, init()).WillOnce(Return(true));
            EXPECT_CALL(led, write(true)).WillOnce(Return(true));
            EXPECT_CALL(pots, init()).WillOnce(Return(true));
            EXPECT_CALL(time, delayMs(100));
            EXPECT_CALL(pots, calibrateCenter(32)).WillOnce(Return(true));
            EXPECT_CALL(serial, writeLine(HasSubstr("Center calibrated")));
        }

        EXPECT_TRUE(app.start());
    }

    TEST(AppControllerTest, StartFailsIfPotentiometerInitFails)
    {
        MockPotentiometerSensor pots;
        MockGpioOutput          led;
        MockSerialIo            serial;
        MockTimeProvider        time;

        domain::AppController app(pots, led, serial, time);

        EXPECT_CALL(serial, init()).WillOnce(Return(true));
        EXPECT_CALL(led, init()).WillOnce(Return(true));
        EXPECT_CALL(led, write(true)).WillOnce(Return(true));
        EXPECT_CALL(pots, init()).WillOnce(Return(false));
        EXPECT_CALL(pots, calibrateCenter(_)).Times(0);

        EXPECT_FALSE(app.start());
    }

    TEST(AppControllerTest, TickProcessesAxesEmitsTelemetryAndTogglesLedEveryInterval)
    {
        MockPotentiometerSensor pots;
        MockGpioOutput          led;
        MockSerialIo            serial;
        MockTimeProvider        time;

        domain::AppConfig cfg{};
        cfg.samplePeriodMs         = 50;
        cfg.ledToggleIntervalTicks = 2;
        for (auto& ax : cfg.axisConfigs)
        {
            ax.deadzoneCounts      = 10;
            ax.maxDeflectionCounts = 1000;
            ax.outputScaleMax      = 500;
        }

        domain::AppController app(pots, led, serial, time, cfg);

        drivers::PotentiometerSnapshot snap{};
        snap.rawCounts  = { 2558, 1538, 2048 };
        snap.millivolts = { 2061, 1239, 1650 };
        snap.rawDelta   = { 510, -510, 5 };

        // Tick 1: no LED toggle yet (tickCount becomes 1, interval is 2)
        EXPECT_CALL(serial, readCharNonBlocking()).WillOnce(Return(-1));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(snap), Return(true)));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)[raw=2558")));
        EXPECT_CALL(led, write(_)).Times(0);
        EXPECT_CALL(time, delayMs(50));

        ASSERT_TRUE(app.tick());
        EXPECT_EQ(app.lastFrame().axes[0].conditionedDelta, 500);
        EXPECT_EQ(app.lastFrame().axes[0].scaled, 250);
        EXPECT_EQ(app.lastFrame().axes[1].conditionedDelta, -500);
        EXPECT_EQ(app.lastFrame().axes[1].scaled, -250);
        EXPECT_EQ(app.lastFrame().axes[2].conditionedDelta, 0);
        EXPECT_EQ(app.lastFrame().axes[2].scaled, 0);

        // Tick 2: LED toggles from true -> false
        EXPECT_CALL(serial, readCharNonBlocking()).WillOnce(Return(-1));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(snap), Return(true)));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)[raw=2558")));
        EXPECT_CALL(led, write(false)).WillOnce(Return(true));
        EXPECT_CALL(time, delayMs(50));

        ASSERT_TRUE(app.tick());
    }

    TEST(AppControllerTest, TickRecalibratesCenterWhenZeroCommandReceived)
    {
        MockPotentiometerSensor pots;
        MockGpioOutput          led;
        MockSerialIo            serial;
        MockTimeProvider        time;

        domain::AppConfig cfg{};
        cfg.calibrationSweeps      = 16;
        cfg.samplePeriodMs         = 20;
        cfg.ledToggleIntervalTicks = 10;

        const std::array<std::uint16_t, 3> kCenters{ 2048, 2048, 2048 };
        ON_CALL(pots, centerCounts()).WillByDefault(ReturnRef(kCenters));

        domain::AppController app(pots, led, serial, time, cfg);

        drivers::PotentiometerSnapshot snap{};
        snap.rawCounts  = { 2048, 2048, 2048 };
        snap.millivolts = { 1650, 1650, 1650 };
        snap.rawDelta   = { 0, 0, 0 };

        EXPECT_CALL(serial, readCharNonBlocking()).WillOnce(Return('z'));
        EXPECT_CALL(pots, calibrateCenter(16)).WillOnce(Return(true));
        EXPECT_CALL(serial, writeLine(HasSubstr("Center calibrated")));
        EXPECT_CALL(pots, readSnapshot(_)).WillOnce(DoAll(SetArgReferee<0>(snap), Return(true)));
        EXPECT_CALL(serial, writeLine(HasSubstr("FWD_BCK(ADC0)[raw=2048")));
        EXPECT_CALL(time, delayMs(20));

        EXPECT_TRUE(app.tick());
    }
}
