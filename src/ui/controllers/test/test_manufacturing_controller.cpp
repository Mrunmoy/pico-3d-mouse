#include "ui/controllers/ManufacturingController.hpp"
#include "ui/mocks/MockFirmwareUpdater.hpp"
#include "ui/mocks/MockSerialTransport.hpp"

#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        using ::testing::InSequence;
        using ::testing::Return;

        TEST(ManufacturingControllerTest, QueriesCalibrationOnConnectAndForwardsSerialLinesToModel)
        {
            mocks::MockSerialTransport serial;
            mocks::MockFirmwareUpdater updater;
            DeviceTelemetryModel       model;
            ManufacturingController    controller{ serial, updater, model };

            EXPECT_CALL(serial, connectPort(QString("/dev/ttyACM0"), 115200))
                .WillOnce(Return(true));
            EXPECT_CALL(serial, sendCommand(QString("GET_CAL")))
                .WillOnce(Return(true));

            EXPECT_TRUE(controller.requestConnect("/dev/ttyACM0", 115200));
            emit serial.connectionStateChanged(true, "/dev/ttyACM0");

            emit serial.lineReceived(
                "FWD_BCK(ADC0)[raw=2117 1706mV d= +0 out= +0]  "
                "LFT_RGT(ADC1)[raw=2111 1701mV d= +0 out= +0]  "
                "ROT(ADC2)[raw=2109 1699mV d= +0 out= +0]");
            EXPECT_EQ(model.axisLive(0).rawCounts, 2117U);
        }

        TEST(ManufacturingControllerTest, SendsCalibrationProtocolCommandsForUserActions)
        {
            mocks::MockSerialTransport serial;
            mocks::MockFirmwareUpdater updater;
            DeviceTelemetryModel       model;
            ManufacturingController    controller{ serial, updater, model };

            {
                InSequence seq;
                EXPECT_CALL(serial, sendCommand(QString("CAL_CENTER"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("CAL_MINMAX_START"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("CAL_MINMAX_END"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("SET_DZ 0 18"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("SET_INV 2 1"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("SAVE_CAL"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("LOAD_CAL"))).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("RESET_CAL"))).WillOnce(Return(true));
            }

            controller.requestZeroCenter();
            controller.requestMinMaxSweepStart();
            controller.requestMinMaxSweepEnd();
            controller.requestSetDeadzone(0, 18);
            controller.requestSetInvert(2, true);
            controller.requestSaveCalibration();
            controller.requestLoadCalibration();
            controller.requestResetCalibration();
        }

        TEST(ManufacturingControllerTest, ButtonlessFirmwareUpdateSendsSoftwareBootselDisconnectsAndStartsUpdater)
        {
            mocks::MockSerialTransport serial;
            mocks::MockFirmwareUpdater updater;
            DeviceTelemetryModel       model;
            ManufacturingController    controller{ serial, updater, model };

            {
                InSequence seq;
                EXPECT_CALL(serial, isConnected()).WillOnce(Return(true));
                EXPECT_CALL(serial, sendCommand(QString("BOOTSEL"))).WillOnce(Return(true));
                EXPECT_CALL(serial, disconnectPort()).Times(1);
                EXPECT_CALL(updater, startUpdate(QString("adc_bringup.uf2"))).WillOnce(Return(true));
            }

            EXPECT_TRUE(controller.requestButtonlessFirmwareUpdate("adc_bringup.uf2"));
        }
    } // namespace
} // namespace p3d::ui
