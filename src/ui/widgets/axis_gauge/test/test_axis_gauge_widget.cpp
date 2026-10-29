#include "ui/widgets/AxisGaugeWidget.hpp"

#include <QApplication>
#include <QPixmap>
#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        QApplication* ensureApp()
        {
            static int   argc = 1;
            static char  arg0[] = "test_axis_gauge_widget";
            static char* argv[] = { arg0, nullptr };
            static QApplication app(argc, argv);
            return &app;
        }

        TEST(AxisGaugeWidgetTest, UpdatesStateClampsCountsAndRendersOffscreen)
        {
            ensureApp();
            AxisGaugeWidget gauge{ "Forward / Backward", "ADC0 / GP26" };

            gauge.setReading(2117, 1706, 5, 1, 150, 3950);
            gauge.setCalibrationMarkers(2112, 16, true);
            gauge.setManufacturingCheckStatus(true, true, false);

            EXPECT_EQ(gauge.rawCounts(), 2117U);
            EXPECT_EQ(gauge.millivolts(), 1706U);
            EXPECT_EQ(gauge.scaledOutput(), 1);
            EXPECT_EQ(gauge.centerCounts(), 2112U);
            EXPECT_EQ(gauge.deadzoneCounts(), 16U);
            EXPECT_FALSE(gauge.manufacturingPassed());

            gauge.setManufacturingCheckStatus(true, true, true);
            EXPECT_TRUE(gauge.manufacturingPassed());

            QPixmap pixmap(gauge.sizeHint());
            gauge.render(&pixmap);
            EXPECT_FALSE(pixmap.isNull());
        }
    } // namespace
} // namespace p3d::ui
