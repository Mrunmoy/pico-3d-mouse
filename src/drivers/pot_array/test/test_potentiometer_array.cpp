#include "drivers/PotentiometerArray.hpp"
#include "drivers/mocks/MockAdcChannelReader.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    using ::testing::_;
    using ::testing::DoAll;
    using ::testing::Return;
    using ::testing::SetArgReferee;

    TEST(PotentiometerArrayTest, InitInitializesAllThreeChannels)
    {
        MockAdcChannelReader ch0;
        MockAdcChannelReader ch1;
        MockAdcChannelReader ch2;

        drivers::PotentiometerArray array(ch0, ch1, ch2);

        EXPECT_CALL(ch0, init()).WillOnce(Return(true));
        EXPECT_CALL(ch1, init()).WillOnce(Return(true));
        EXPECT_CALL(ch2, init()).WillOnce(Return(true));

        EXPECT_TRUE(array.init());
    }

    TEST(PotentiometerArrayTest, InitFailsFastIfChannelInitFails)
    {
        MockAdcChannelReader ch0;
        MockAdcChannelReader ch1;
        MockAdcChannelReader ch2;

        drivers::PotentiometerArray array(ch0, ch1, ch2);

        EXPECT_CALL(ch0, init()).WillOnce(Return(true));
        EXPECT_CALL(ch1, init()).WillOnce(Return(false));
        EXPECT_CALL(ch2, init()).Times(0);

        EXPECT_FALSE(array.init());
    }

    TEST(PotentiometerArrayTest, CalibrateCenterAveragesSweepsAndReadComputesSignedDelta)
    {
        MockAdcChannelReader ch0;
        MockAdcChannelReader ch1;
        MockAdcChannelReader ch2;

        drivers::PotentiometerArray array(ch0, ch1, ch2);

        // 2 calibration sweeps:
        // ch0: 2000, 2020 -> center = 2010
        // ch1: 1900, 1940 -> center = 1920
        // ch2: 2100, 2100 -> center = 2100
        EXPECT_CALL(ch0, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2000, 1611 }), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2020, 1627 }), Return(true)));
        EXPECT_CALL(ch1, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 1900, 1531 }), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 1940, 1563 }), Return(true)));
        EXPECT_CALL(ch2, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2100, 1692 }), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2100, 1692 }), Return(true)));

        ASSERT_TRUE(array.calibrateCenter(2));
        EXPECT_EQ(array.centerCounts()[0], 2010U);
        EXPECT_EQ(array.centerCounts()[1], 1920U);
        EXPECT_EQ(array.centerCounts()[2], 2100U);

        // Now read a live snapshot:
        // ch0 = 2510 (+500), ch1 = 1520 (-400), ch2 = 2100 (0)
        EXPECT_CALL(ch0, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2510, 2022 }), Return(true)));
        EXPECT_CALL(ch1, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 1520, 1224 }), Return(true)));
        EXPECT_CALL(ch2, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2100, 1692 }), Return(true)));

        drivers::PotentiometerSnapshot snap{};
        ASSERT_TRUE(array.readSnapshot(snap));

        EXPECT_EQ(snap.rawCounts[0], 2510U);
        EXPECT_EQ(snap.millivolts[0], 2022U);
        EXPECT_EQ(snap.rawDelta[0], 500);

        EXPECT_EQ(snap.rawCounts[1], 1520U);
        EXPECT_EQ(snap.millivolts[1], 1224U);
        EXPECT_EQ(snap.rawDelta[1], -400);

        EXPECT_EQ(snap.rawCounts[2], 2100U);
        EXPECT_EQ(snap.millivolts[2], 1692U);
        EXPECT_EQ(snap.rawDelta[2], 0);
    }

    TEST(PotentiometerArrayTest, PropagatesChannelReadFailure)
    {
        MockAdcChannelReader ch0;
        MockAdcChannelReader ch1;
        MockAdcChannelReader ch2;

        drivers::PotentiometerArray array(ch0, ch1, ch2);

        EXPECT_CALL(ch0, read(_))
            .WillOnce(DoAll(SetArgReferee<0>(drivers::AdcSample{ 2048, 1650 }), Return(true)));
        EXPECT_CALL(ch1, read(_)).WillOnce(Return(false));
        EXPECT_CALL(ch2, read(_)).Times(0);

        drivers::PotentiometerSnapshot snap{};
        EXPECT_FALSE(array.readSnapshot(snap));
    }
}
