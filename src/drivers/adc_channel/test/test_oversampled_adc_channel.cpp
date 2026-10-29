#include "drivers/OversampledAdcChannel.hpp"
#include "hal/mocks/MockAdcHardware.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    using ::testing::_;
    using ::testing::DoAll;
    using ::testing::InSequence;
    using ::testing::Return;
    using ::testing::SetArgReferee;

    TEST(OversampledAdcChannelTest, InitConfiguresConfiguredChannel)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex = 2;

        drivers::OversampledAdcChannel ch(hw, cfg);

        EXPECT_CALL(hw, initChannel(2)).WillOnce(Return(true));
        EXPECT_TRUE(ch.init());

        EXPECT_CALL(hw, initChannel(2)).WillOnce(Return(false));
        EXPECT_FALSE(ch.init());
    }

    TEST(OversampledAdcChannelTest, ReadDiscardsSettlingConversionAndAveragesOversamples)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex        = 1;
        cfg.oversampleCount     = 4;
        cfg.discardSettlingRead = true;
        cfg.vrefMillivolts      = 3300;
        cfg.maxRawCounts        = 4095;

        drivers::OversampledAdcChannel ch(hw, cfg);

        {
            InSequence seq;
            EXPECT_CALL(hw, selectChannel(1)).WillOnce(Return(true));
            // Dummy settling read (value 9999 should be ignored)
            EXPECT_CALL(hw, readRaw(_)).WillOnce(DoAll(SetArgReferee<0>(4095), Return(true)));
            // 4 oversampled reads: 2000, 2040, 2060, 2100 -> average = 2050
            EXPECT_CALL(hw, readRaw(_)).WillOnce(DoAll(SetArgReferee<0>(2000), Return(true)));
            EXPECT_CALL(hw, readRaw(_)).WillOnce(DoAll(SetArgReferee<0>(2040), Return(true)));
            EXPECT_CALL(hw, readRaw(_)).WillOnce(DoAll(SetArgReferee<0>(2060), Return(true)));
            EXPECT_CALL(hw, readRaw(_)).WillOnce(DoAll(SetArgReferee<0>(2100), Return(true)));
        }

        drivers::AdcSample sample{};
        ASSERT_TRUE(ch.read(sample));
        EXPECT_EQ(sample.rawCounts, 2050U);
        EXPECT_EQ(sample.millivolts, static_cast<std::uint16_t>((2050U * 3300U) / 4095U));
    }

    TEST(OversampledAdcChannelTest, ReadWithoutSettlingReadDoesNotDiscardFirstConversion)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex        = 0;
        cfg.oversampleCount     = 2;
        cfg.discardSettlingRead = false;

        drivers::OversampledAdcChannel ch(hw, cfg);

        EXPECT_CALL(hw, selectChannel(0)).WillOnce(Return(true));
        EXPECT_CALL(hw, readRaw(_))
            .WillOnce(DoAll(SetArgReferee<0>(1000), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(3000), Return(true)));

        drivers::AdcSample sample{};
        ASSERT_TRUE(ch.read(sample));
        EXPECT_EQ(sample.rawCounts, 2000U);
    }

    TEST(OversampledAdcChannelTest, ZeroOversampleCountDefaultsToSingleSample)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex        = 0;
        cfg.oversampleCount     = 0;
        cfg.discardSettlingRead = false;

        drivers::OversampledAdcChannel ch(hw, cfg);

        EXPECT_CALL(hw, selectChannel(0)).WillOnce(Return(true));
        EXPECT_CALL(hw, readRaw(_)).WillOnce(DoAll(SetArgReferee<0>(4095), Return(true)));

        drivers::AdcSample sample{};
        ASSERT_TRUE(ch.read(sample));
        EXPECT_EQ(sample.rawCounts, 4095U);
        EXPECT_EQ(sample.millivolts, 3300U);
    }

    TEST(OversampledAdcChannelTest, PropagatesSelectChannelFailure)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex = 2;

        drivers::OversampledAdcChannel ch(hw, cfg);

        EXPECT_CALL(hw, selectChannel(2)).WillOnce(Return(false));
        EXPECT_CALL(hw, readRaw(_)).Times(0);

        drivers::AdcSample sample{};
        EXPECT_FALSE(ch.read(sample));
    }

    TEST(OversampledAdcChannelTest, PropagatesSettlingReadFailure)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex        = 0;
        cfg.oversampleCount     = 4;
        cfg.discardSettlingRead = true;

        drivers::OversampledAdcChannel ch(hw, cfg);

        EXPECT_CALL(hw, selectChannel(0)).WillOnce(Return(true));
        EXPECT_CALL(hw, readRaw(_)).WillOnce(Return(false));

        drivers::AdcSample sample{};
        EXPECT_FALSE(ch.read(sample));
    }

    TEST(OversampledAdcChannelTest, PropagatesSampleReadFailureMidLoop)
    {
        MockAdcHardware hw;
        drivers::AdcChannelConfig cfg{};
        cfg.channelIndex        = 0;
        cfg.oversampleCount     = 4;
        cfg.discardSettlingRead = false;

        drivers::OversampledAdcChannel ch(hw, cfg);

        EXPECT_CALL(hw, selectChannel(0)).WillOnce(Return(true));
        EXPECT_CALL(hw, readRaw(_))
            .WillOnce(DoAll(SetArgReferee<0>(1024), Return(true)))
            .WillOnce(Return(false));

        drivers::AdcSample sample{};
        EXPECT_FALSE(ch.read(sample));
    }
}
