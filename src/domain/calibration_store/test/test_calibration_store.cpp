#include "domain/CalibrationStore.hpp"
#include "hal/mocks/MockNvmStorage.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    using ::testing::_;
    using ::testing::Return;

    TEST(CalibrationStoreTest, SaveAndLoadRoundTripPreservesAllAxisSettings)
    {
        MockNvmStorage nvm;
        nvm.useBackingStore(256);

        domain::CalibrationStore store(nvm);

        domain::CalibrationData written{};
        written.centerCounts   = { 2116, 2111, 2109 };
        written.minCounts      = { 150, 200, 180 };
        written.maxCounts      = { 3950, 3900, 4010 };
        written.deadzoneCounts = { 15, 18, 12 };
        written.invert         = { true, false, true };

        ASSERT_TRUE(store.save(written));

        domain::CalibrationData loaded{};
        ASSERT_TRUE(store.load(loaded));

        EXPECT_EQ(loaded.centerCounts, written.centerCounts);
        EXPECT_EQ(loaded.minCounts, written.minCounts);
        EXPECT_EQ(loaded.maxCounts, written.maxCounts);
        EXPECT_EQ(loaded.deadzoneCounts, written.deadzoneCounts);
        EXPECT_EQ(loaded.invert, written.invert);
    }

    TEST(CalibrationStoreTest, LoadFailsOnBlankErasedFlash)
    {
        MockNvmStorage nvm;
        nvm.useBackingStore(256);  // Initialized to 0xFF (erased NOR flash)

        domain::CalibrationStore store(nvm);

        domain::CalibrationData out{};
        EXPECT_FALSE(store.load(out));
    }

    TEST(CalibrationStoreTest, LoadFailsWhenBitCorruptedInStorageDueToCrc32)
    {
        MockNvmStorage nvm;
        nvm.useBackingStore(256);

        domain::CalibrationStore store(nvm);

        domain::CalibrationData written{};
        ASSERT_TRUE(store.save(written));

        // Corrupt a single payload byte in the backing flash
        nvm.rawStore()[10] ^= 0x04U;

        domain::CalibrationData out{};
        EXPECT_FALSE(store.load(out));
    }

    TEST(CalibrationStoreTest, RejectsInvalidCalibrationRanges)
    {
        MockNvmStorage nvm;
        nvm.useBackingStore(256);

        domain::CalibrationStore store(nvm);

        domain::CalibrationData badMin{};
        badMin.centerCounts = { 2000, 2000, 2000 };
        badMin.minCounts    = { 2000, 100, 100 };  // min >= center on axis 0
        EXPECT_FALSE(store.save(badMin));

        domain::CalibrationData badMax{};
        badMax.centerCounts = { 2000, 2000, 2000 };
        badMax.maxCounts    = { 4095, 5000, 4095 };  // max > 4095 (12-bit ADC)
        EXPECT_FALSE(store.save(badMax));
    }

    TEST(CalibrationStoreTest, PropagatesNvmReadAndWriteFailures)
    {
        MockNvmStorage nvm;
        domain::CalibrationStore store(nvm);

        EXPECT_CALL(nvm, write(_, _)).WillOnce(Return(false));
        domain::CalibrationData d{};
        EXPECT_FALSE(store.save(d));

        EXPECT_CALL(nvm, read(_, _)).WillOnce(Return(false));
        EXPECT_FALSE(store.load(d));
    }
}
