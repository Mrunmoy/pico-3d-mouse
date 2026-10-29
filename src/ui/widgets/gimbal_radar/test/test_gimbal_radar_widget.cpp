#include "ui/widgets/GimbalRadarWidget.hpp"

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
            static char  arg0[] = "test_gimbal_radar_widget";
            static char* argv[] = { arg0, nullptr };
            static QApplication app(argc, argv);
            return &app;
        }

        TEST(GimbalRadarWidgetTest, ComputesNormalizedDeflectionAndClampsToUnitRange)
        {
            ensureApp();
            GimbalRadarWidget widget;

            widget.setAxesOutput(250, -500, 125, 500);
            EXPECT_EQ(widget.forwardBackward(), 250);
            EXPECT_EQ(widget.leftRight(), -500);
            EXPECT_EQ(widget.rotation(), 125);
            EXPECT_DOUBLE_EQ(widget.normalizedDeflection().x(), -1.0);
            EXPECT_DOUBLE_EQ(widget.normalizedDeflection().y(), 0.5);
            EXPECT_DOUBLE_EQ(widget.normalizedRotation(), 0.25);

            // Clamps out-of-range inputs cleanly and renders offscreen without error
            widget.setAxesOutput(900, -900, 900, 500);
            EXPECT_DOUBLE_EQ(widget.normalizedDeflection().x(), -1.0);
            EXPECT_DOUBLE_EQ(widget.normalizedDeflection().y(), 1.0);
            EXPECT_DOUBLE_EQ(widget.normalizedRotation(), 1.0);

            QPixmap pixmap(widget.sizeHint());
            widget.render(&pixmap);
            EXPECT_FALSE(pixmap.isNull());
        }
    } // namespace
} // namespace p3d::ui
