#include "ui/mocks/MockProcessRunner.hpp"
#include "ui/services/PicotoolFirmwareUpdater.hpp"
#include "ui/services/QtSerialTransport.hpp"

#include <QStringList>
#include <gtest/gtest.h>

namespace p3d::ui
{
    namespace
    {
        using ::testing::_;
        using ::testing::Return;

        TEST(UiServicesTest, QtSerialTransportFramesArbitraryByteChunksIntoLines)
        {
            QtSerialTransport transport;
            QStringList       lines;

            QObject::connect(&transport,
                             &ISerialTransport::lineReceived,
                             [&](const QString& line) { lines.append(line); });

            // Feed partial fragments, CRLF endings, and empty lines
            transport.ingestRawBytes("FWD_BCK(ADC0)[raw=2117 ");
            EXPECT_TRUE(lines.isEmpty());

            transport.ingestRawBytes("1706mV d=   +0 out=  +0]\r\n\r\n@CAL STATUS ");
            ASSERT_EQ(lines.size(), 1);
            EXPECT_EQ(lines[0], "FWD_BCK(ADC0)[raw=2117 1706mV d=   +0 out=  +0]");

            transport.ingestRawBytes("CH0[c=2117 min=0 max=4095 dz=12 inv=0]\n");
            ASSERT_EQ(lines.size(), 2);
            EXPECT_EQ(lines[1], "@CAL STATUS CH0[c=2117 min=0 max=4095 dz=12 inv=0]");
        }

        TEST(UiServicesTest, QtSerialTransportFailsCleanlyWhenSendingWhileDisconnected)
        {
            QtSerialTransport transport;
            QString           errMsg;

            QObject::connect(&transport,
                             &ISerialTransport::errorOccurred,
                             [&](const QString& msg) { errMsg = msg; });

            EXPECT_FALSE(transport.isConnected());
            EXPECT_FALSE(transport.sendCommand("GET_CAL"));
            EXPECT_FALSE(errMsg.isEmpty());
        }

        TEST(UiServicesTest, PicotoolFirmwareUpdaterRunsButtonlessPicotoolLoadAndEmitsSuccess)
        {
            mocks::MockProcessRunner mockRunner;
            PicotoolFirmwareUpdater  updater{ mockRunner, /*verifyFileExists=*/false };

            QStringList logs;
            bool        finishedCalled = false;
            bool        finishedOk     = false;

            QObject::connect(&updater,
                             &IFirmwareUpdater::progressLog,
                             [&](const QString& line) { logs.append(line); });
            QObject::connect(&updater,
                             &IFirmwareUpdater::updateFinished,
                             [&](bool ok, const QString&) {
                                 finishedCalled = true;
                                 finishedOk     = ok;
                             });

            EXPECT_CALL(mockRunner, isRunning()).WillOnce(Return(false));
            EXPECT_CALL(mockRunner,
                        startProcess(QString("picotool"),
                                     QStringList{ "load", "-f", "-v", "-x", "/tmp/adc_bringup.uf2" }))
                .Times(1);

            EXPECT_TRUE(updater.startUpdate("/tmp/adc_bringup.uf2"));
            EXPECT_TRUE(updater.isUpdating());

            emit mockRunner.outputProduced("Loading into Flash: 100%\nVerifying Flash: OK\n");
            emit mockRunner.processFinished(0, false);

            EXPECT_FALSE(updater.isUpdating());
            EXPECT_TRUE(finishedCalled);
            EXPECT_TRUE(finishedOk);
            EXPECT_GE(logs.size(), 4);
        }

        TEST(UiServicesTest, PicotoolFirmwareUpdaterRejectsNonUf2PathAndReportsFailureExitCode)
        {
            mocks::MockProcessRunner mockRunner;
            PicotoolFirmwareUpdater  updater{ mockRunner, /*verifyFileExists=*/false };

            bool finishedOk = true;
            QObject::connect(&updater,
                             &IFirmwareUpdater::updateFinished,
                             [&](bool ok, const QString&) { finishedOk = ok; });

            EXPECT_FALSE(updater.startUpdate("not_a_uf2.bin"));
            EXPECT_FALSE(finishedOk);

            EXPECT_CALL(mockRunner, isRunning()).WillOnce(Return(false));
            EXPECT_CALL(mockRunner, startProcess(_, _)).Times(1);

            EXPECT_TRUE(updater.startUpdate("firmware.uf2"));
            emit mockRunner.processFinished(249, false);
            EXPECT_FALSE(finishedOk);
        }
    } // namespace
} // namespace p3d::ui
