#include "domain/AxisProcessor.hpp"

#include <gtest/gtest.h>

namespace p3d::test
{
    TEST(AxisProcessorTest, SuppressesValuesWithinSymmetricDeadzone)
    {
        domain::AxisConfig cfg{};
        cfg.deadzoneCounts      = 15;
        cfg.maxDeflectionCounts = 1000;
        cfg.outputScaleMax      = 500;
        cfg.emaAlpha            = 1.0f;

        domain::AxisProcessor proc(cfg);

        for (std::int16_t d : { static_cast<std::int16_t>(0),
                                static_cast<std::int16_t>(15),
                                static_cast<std::int16_t>(-15),
                                static_cast<std::int16_t>(7),
                                static_cast<std::int16_t>(-14) })
        {
            const auto out = proc.process(d);
            EXPECT_EQ(out.conditionedDelta, 0);
            EXPECT_FLOAT_EQ(out.normalized, 0.0f);
            EXPECT_EQ(out.scaled, 0);
        }
    }

    TEST(AxisProcessorTest, ContinuousTransitionOutsideDeadzone)
    {
        domain::AxisConfig cfg{};
        cfg.deadzoneCounts      = 10;
        cfg.maxDeflectionCounts = 100;
        cfg.outputScaleMax      = 100;
        cfg.emaAlpha            = 1.0f;

        domain::AxisProcessor proc(cfg);

        const auto pos = proc.process(35);  // 35 - 10 = +25 -> +0.25 -> +25
        EXPECT_EQ(pos.conditionedDelta, 25);
        EXPECT_NEAR(pos.normalized, 0.25f, 1e-5f);
        EXPECT_EQ(pos.scaled, 25);

        const auto neg = proc.process(-60);  // -60 + 10 = -50 -> -0.50 -> -50
        EXPECT_EQ(neg.conditionedDelta, -50);
        EXPECT_NEAR(neg.normalized, -0.50f, 1e-5f);
        EXPECT_EQ(neg.scaled, -50);
    }

    TEST(AxisProcessorTest, InvertsPolarityWhenConfigured)
    {
        domain::AxisConfig cfg{};
        cfg.deadzoneCounts      = 10;
        cfg.maxDeflectionCounts = 100;
        cfg.outputScaleMax      = 200;
        cfg.invert              = true;

        domain::AxisProcessor proc(cfg);

        const auto out = proc.process(60);  // +(60 - 10) inverted -> -50 -> -0.5 -> -100
        EXPECT_EQ(out.conditionedDelta, -50);
        EXPECT_NEAR(out.normalized, -0.5f, 1e-5f);
        EXPECT_EQ(out.scaled, -100);
    }

    TEST(AxisProcessorTest, ClampsAtFullScaleDeflection)
    {
        domain::AxisConfig cfg{};
        cfg.deadzoneCounts      = 10;
        cfg.maxDeflectionCounts = 500;
        cfg.outputScaleMax      = 350;

        domain::AxisProcessor proc(cfg);

        const auto hi = proc.process(2000);
        EXPECT_FLOAT_EQ(hi.normalized, 1.0f);
        EXPECT_EQ(hi.scaled, 350);

        const auto lo = proc.process(-2000);
        EXPECT_FLOAT_EQ(lo.normalized, -1.0f);
        EXPECT_EQ(lo.scaled, -350);
    }

    TEST(AxisProcessorTest, AppliesEmaFilterAndResetClearsHistory)
    {
        domain::AxisConfig cfg{};
        cfg.deadzoneCounts      = 0;
        cfg.maxDeflectionCounts = 100;
        cfg.outputScaleMax      = 100;
        cfg.emaAlpha            = 0.5f;

        domain::AxisProcessor proc(cfg);

        // First sample after construction seeds the filter directly
        const auto s1 = proc.process(0);
        EXPECT_NEAR(s1.normalized, 0.0f, 1e-5f);

        // Step input to +100 (norm = 1.0): filtered = 0.5 * 1.0 + 0.5 * 0.0 = 0.5
        const auto s2 = proc.process(100);
        EXPECT_NEAR(s2.normalized, 0.5f, 1e-5f);
        EXPECT_EQ(s2.scaled, 50);

        // Next sample at +100: filtered = 0.5 * 1.0 + 0.5 * 0.5 = 0.75
        const auto s3 = proc.process(100);
        EXPECT_NEAR(s3.normalized, 0.75f, 1e-5f);
        EXPECT_EQ(s3.scaled, 75);

        // Reset clears filter state
        proc.reset();
        const auto s4 = proc.process(0);
        EXPECT_NEAR(s4.normalized, 0.0f, 1e-5f);
        EXPECT_EQ(s4.scaled, 0);
    }
}
