#include "ui/mocks/MockFirmwareUpdater.hpp"
#include "ui/mocks/MockProcessRunner.hpp"
#include "ui/mocks/MockSerialTransport.hpp"

#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        using ::testing::Return;

        TEST(UiInterfacesTest, SerialTransportSignalsAndPolymorphicCalls)
        {
            mocks::MockSerialTransport mockSerial;
            QString                    receivedLine;

            QObject::connect(&mockSerial,
                             &ISerialTransport::lineReceived,
                             [&](const QString& line) { receivedLine = line; });

            EXPECT_CALL(mockSerial, connectPort(QString("/dev/ttyACM0"), 115200))
                .WillOnce(Return(true));

            ISerialTransport& iface = mockSerial;
            EXPECT_TRUE(iface.connectPort("/dev/ttyACM0", 115200));

            emit mockSerial.lineReceived("FWD_BCK(ADC0)[raw=2048 1650mV d=   +0 out=  +0]");
            EXPECT_EQ(receivedLine, "FWD_BCK(ADC0)[raw=2048 1650mV d=   +0 out=  +0]");
        }

        TEST(UiInterfacesTest, FirmwareUpdaterSignalsAndPolymorphicCalls)
        {
            mocks::MockFirmwareUpdater mockUpdater;
            bool                       finishedOk = false;

            QObject::connect(&mockUpdater,
                             &IFirmwareUpdater::updateFinished,
                             [&](bool ok, const QString&) { finishedOk = ok; });

            EXPECT_CALL(mockUpdater, startUpdate(QString("adc_bringup.uf2")))
                .WillOnce(Return(true));

            IFirmwareUpdater& iface = mockUpdater;
            EXPECT_TRUE(iface.startUpdate("adc_bringup.uf2"));

            emit mockUpdater.updateFinished(true, "Flashed successfully");
            EXPECT_TRUE(finishedOk);
        }
    } // namespace
} // namespace p3d::ui
