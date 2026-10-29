#include "ui/widgets/CalibrationPanelWidget.hpp"

#include <QApplication>
#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        QApplication* ensureApp()
        {
            static int   argc = 1;
            static char  arg0[] = "test_calibration_panel_widget";
            static char* argv[] = { arg0, nullptr };
            static QApplication app(argc, argv);
            return &app;
        }

        TEST(CalibrationPanelWidgetTest, EmitsSignalsOnButtonClicksAndBlocksFeedbackLoopOnSetAxisCalibration)
        {
            ensureApp();
            CalibrationPanelWidget panel;

            bool zeroClicked  = false;
            bool startClicked = false;
            bool endClicked   = false;
            bool saveClicked  = false;
            bool loadClicked  = false;
            bool resetClicked = false;
            int  dzEmitCount  = 0;

            QObject::connect(&panel, &CalibrationPanelWidget::zeroCenterRequested, [&]() { zeroClicked = true; });
            QObject::connect(&panel, &CalibrationPanelWidget::minMaxSweepStartRequested, [&]() { startClicked = true; });
            QObject::connect(&panel, &CalibrationPanelWidget::minMaxSweepEndRequested, [&]() { endClicked = true; });
            QObject::connect(&panel, &CalibrationPanelWidget::saveCalibrationRequested, [&]() { saveClicked = true; });
            QObject::connect(&panel, &CalibrationPanelWidget::loadCalibrationRequested, [&]() { loadClicked = true; });
            QObject::connect(&panel, &CalibrationPanelWidget::resetCalibrationRequested, [&]() { resetClicked = true; });
            QObject::connect(&panel, &CalibrationPanelWidget::deadzoneChanged, [&](int, int) { ++dzEmitCount; });

            panel.zeroCenterButton()->click();
            panel.startSweepButton()->click();
            panel.endSweepButton()->click();
            panel.saveFlashButton()->click();
            panel.loadFlashButton()->click();
            panel.resetFactoryButton()->click();

            EXPECT_TRUE(zeroClicked);
            EXPECT_TRUE(startClicked);
            EXPECT_TRUE(endClicked);
            EXPECT_TRUE(saveClicked);
            EXPECT_TRUE(loadClicked);
            EXPECT_TRUE(resetClicked);

            // Programmatic update via setAxisCalibration must NOT emit deadzoneChanged feedback signal
            panel.setAxisCalibration(0, 2117, 100, 4000, 25, true);
            EXPECT_EQ(panel.deadzoneForAxis(0), 25);
            EXPECT_TRUE(panel.invertForAxis(0));
            EXPECT_EQ(dzEmitCount, 0);
        }
    } // namespace
} // namespace p3d::ui
