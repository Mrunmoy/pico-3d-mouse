#include "ui/app/MainWindow.hpp"
#include "ui/mocks/MockFirmwareUpdater.hpp"
#include "ui/mocks/MockSerialTransport.hpp"

#include <QApplication>
#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        using ::testing::InSequence;
        using ::testing::Return;

        QApplication* ensureApp()
        {
            static int   argc = 1;
            static char  arg0[] = "test_main_window";
            static char* argv[] = { arg0, nullptr };
            static QApplication app(argc, argv);
            return &app;
        }

        TEST(MainWindowTest, EndToEndReactiveMvcFlowUpdatesAllCustomWidgetsAndTriggersButtonlessUpdate)
        {
            ensureApp();

            mocks::MockSerialTransport serial;
            mocks::MockFirmwareUpdater updater;
            DeviceTelemetryModel       model;
            ManufacturingController    controller{ serial, updater, model };

            EXPECT_CALL(serial, availablePorts())
                .WillOnce(Return(QStringList{ "/dev/ttyACM0" }));

            MainWindow window{ serial, updater, model, controller };

            // Starts on Wizard Step 0 (ConnectDevice)
            EXPECT_EQ(window.manufacturingWizard()->currentStep(),
                      ManufacturingWizardWidget::Step::ConnectDevice);

            // Simulate connection -> automatically queries GET_CAL and advances Wizard to Step 1 (CenterCalibration)
            EXPECT_CALL(serial, sendCommand(QString("GET_CAL"))).WillOnce(Return(true));
            emit serial.connectionStateChanged(true, "/dev/ttyACM0");
            EXPECT_EQ(window.manufacturingWizard()->currentStep(),
                      ManufacturingWizardWidget::Step::CenterCalibration);

            // 1. Emit live telemetry from device -> verify AxisGaugeWidgets and GimbalRadarWidget update reactively
            emit serial.lineReceived(
                "FWD_BCK(ADC0)[raw=2048 1650mV d= +0 out= +0]  "
                "LFT_RGT(ADC1)[raw=2050 1651mV d= +0 out= +0]  "
                "ROT(ADC2)[raw=2045 1647mV d= +0 out= +0]");

            EXPECT_EQ(window.axisGauge(0)->rawCounts(), 2048U);
            EXPECT_EQ(window.axisGauge(1)->rawCounts(), 2050U);
            EXPECT_EQ(window.axisGauge(2)->rawCounts(), 2045U);

            // 2. Sweep negative and positive mechanical limits -> verify Manufacturing Verdict badge turns PASS
            emit serial.lineReceived(
                "FWD_BCK(ADC0)[raw=200 161mV d=-1848 out=-500]  "
                "LFT_RGT(ADC1)[raw=220 177mV d=-1830 out=-500]  "
                "ROT(ADC2)[raw=180 145mV d=-1865 out=-500]");
            emit serial.lineReceived(
                "FWD_BCK(ADC0)[raw=3900 3142mV d=+1852 out=+500]  "
                "LFT_RGT(ADC1)[raw=3920 3158mV d=+1870 out=+500]  "
                "ROT(ADC2)[raw=3880 3126mV d=+1835 out=+500]");

            EXPECT_TRUE(window.axisGauge(0)->manufacturingPassed());
            EXPECT_TRUE(window.axisGauge(1)->manufacturingPassed());
            EXPECT_TRUE(window.axisGauge(2)->manufacturingPassed());
            EXPECT_TRUE(window.overallVerdictBadge()->text().contains("PASS"));
            EXPECT_EQ(window.gimbalRadar()->forwardBackward(), 500);

            // 3. Emit @CAL SAVED_OK -> verify CalibrationPanelWidget, Wizard, and AxisGaugeWidget markers update
            emit serial.lineReceived(
                "@CAL SAVED_OK CH0[c=2117 min=100 max=4000 dz=18 inv=1] "
                "CH1[c=2111 min=110 max=3990 dz=12 inv=0] "
                "CH2[c=2109 min=120 max=3980 dz=14 inv=0]");

            EXPECT_EQ(window.calibrationPanel()->deadzoneForAxis(0), 18);
            EXPECT_TRUE(window.calibrationPanel()->invertForAxis(0));
            EXPECT_EQ(window.manufacturingWizard()->deadzoneForAxis(0), 18);
            EXPECT_TRUE(window.manufacturingWizard()->invertForAxis(0));
            EXPECT_EQ(window.axisGauge(0)->centerCounts(), 2117U);

            // 4. Click "Save to QSPI Flash" on CalibrationPanelWidget -> sends SAVE_CAL
            EXPECT_CALL(serial, sendCommand(QString("SAVE_CAL"))).WillOnce(Return(true));
            window.calibrationPanel()->saveFlashButton()->click();

            // 5. Click "Flash Firmware Over USB (Buttonless)" -> sends BOOTSEL, disconnects serial, starts updater
            {
                InSequence seq;
                EXPECT_CALL(serial, isConnected()).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("BOOTSEL"))).WillOnce(Return(true));
                EXPECT_CALL(serial, disconnectPort()).Times(1);
                EXPECT_CALL(
                    updater,
                    startUpdate(QString("firmware/build/apps/pico_3d_mouse/pico_3d_mouse.uf2")))
                    .WillOnce(Return(true));
            }
            window.firmwareUpdaterWidget()->flashButton()->click();
        }
    } // namespace
} // namespace p3d::ui
