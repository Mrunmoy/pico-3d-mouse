#include "ui/widgets/ManufacturingWizardWidget.hpp"

#include <QApplication>
#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        QApplication* ensureApp()
        {
            static int   argc = 1;
            static char  arg0[] = "test_manufacturing_wizard_widget";
            static char* argv[] = { arg0, nullptr };
            static QApplication app(argc, argv);
            return &app;
        }

        TEST(ManufacturingWizardWidgetTest, WalksThroughAllSixManufacturingStepsReactively)
        {
            ensureApp();
            ManufacturingWizardWidget wizard;

            bool zeroRequested      = false;
            bool sweepStartEmitted  = false;
            bool sweepEndRequested  = false;
            bool saveFlashRequested = false;

            QObject::connect(&wizard,
                             &ManufacturingWizardWidget::zeroCenterRequested,
                             [&]() { zeroRequested = true; });
            QObject::connect(&wizard,
                             &ManufacturingWizardWidget::minMaxSweepStartRequested,
                             [&]() { sweepStartEmitted = true; });
            QObject::connect(&wizard,
                             &ManufacturingWizardWidget::minMaxSweepEndRequested,
                             [&]() { sweepEndRequested = true; });
            QObject::connect(&wizard,
                             &ManufacturingWizardWidget::saveCalibrationRequested,
                             [&]() { saveFlashRequested = true; });

            // Starts at Step 0 (ConnectDevice)
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::ConnectDevice);

            // Connecting advances automatically to Step 1 (CenterCalibration)
            wizard.setConnected(true, "/dev/ttyACM0");
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::CenterCalibration);

            // Clicking Capture Center emits zeroCenterRequested; ZERO_OK advances to Step 2 (RangeSweep)
            wizard.captureCenterButton()->click();
            EXPECT_TRUE(zeroRequested);
            wizard.onDeviceCalibrationEvent("ZERO_OK");
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::RangeSweep);
            EXPECT_TRUE(sweepStartEmitted);

            // Clicking Finish Sweep emits minMaxSweepEndRequested; MINMAX_END advances to Step 3 (VerifyAndTune)
            wizard.finishSweepButton()->click();
            EXPECT_TRUE(sweepEndRequested);
            wizard.onDeviceCalibrationEvent("MINMAX_END");
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::VerifyAndTune);

            // Clicking Confirm Tuning advances to Step 4 (SaveToFlash)
            wizard.confirmTuningButton()->click();
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::SaveToFlash);

            // Clicking Save to Flash emits saveCalibrationRequested; SAVED_OK advances to Step 5 (CompletedPass)
            wizard.saveToFlashButton()->click();
            EXPECT_TRUE(saveFlashRequested);
            wizard.onDeviceCalibrationEvent("SAVED_OK");
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::CompletedPass);

            // Clicking Test Next Unit returns to Step 1 (CenterCalibration) while still connected
            wizard.testNextUnitButton()->click();
            EXPECT_EQ(wizard.currentStep(), ManufacturingWizardWidget::Step::CenterCalibration);
        }
    } // namespace
} // namespace p3d::ui
