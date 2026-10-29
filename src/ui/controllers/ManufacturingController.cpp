#include "ui/controllers/ManufacturingController.hpp"

namespace p3d::ui
{
    ManufacturingController::ManufacturingController(ISerialTransport&     serialTransport,
                                                     IFirmwareUpdater&     firmwareUpdater,
                                                     DeviceTelemetryModel& telemetryModel,
                                                     QObject*              parent)
        : QObject(parent)
        , m_serial(serialTransport)
        , m_updater(firmwareUpdater)
        , m_model(telemetryModel)
    {
        connect(&m_serial,
                &ISerialTransport::lineReceived,
                &m_model,
                &DeviceTelemetryModel::onSerialLineReceived);
        connect(&m_serial,
                &ISerialTransport::connectionStateChanged,
                this,
                &ManufacturingController::onConnectionStateChanged);
        connect(&m_serial,
                &ISerialTransport::errorOccurred,
                this,
                &ManufacturingController::statusAnnouncement);
    }

    bool ManufacturingController::requestConnect(const QString& portName, qint32 baudRate)
    {
        return m_serial.connectPort(portName, baudRate);
    }

    void ManufacturingController::requestDisconnect()
    {
        m_serial.disconnectPort();
    }

    void ManufacturingController::requestZeroCenter()
    {
        if (m_serial.sendCommand(QStringLiteral("ZERO")))
        {
            emit statusAnnouncement(QStringLiteral("Sent ZERO command to calibrate resting center."));
        }
    }

    void ManufacturingController::requestMinMaxSweepStart()
    {
        if (m_serial.sendCommand(QStringLiteral("CAL_MINMAX_START")))
        {
            emit statusAnnouncement(
                QStringLiteral("Min/Max sweep started — move all 3 axes to full mechanical limits."));
        }
    }

    void ManufacturingController::requestMinMaxSweepEnd()
    {
        if (m_serial.sendCommand(QStringLiteral("CAL_MINMAX_END")))
        {
            emit statusAnnouncement(QStringLiteral("Min/Max sweep finished and applied."));
        }
    }

    void ManufacturingController::requestSaveCalibration()
    {
        if (m_serial.sendCommand(QStringLiteral("SAVE_CAL")))
        {
            emit statusAnnouncement(
                QStringLiteral("Saving calibration to RP2040 internal QSPI Flash..."));
        }
    }

    void ManufacturingController::requestLoadCalibration()
    {
        if (m_serial.sendCommand(QStringLiteral("LOAD_CAL")))
        {
            emit statusAnnouncement(
                QStringLiteral("Loading calibration from RP2040 internal QSPI Flash..."));
        }
    }

    void ManufacturingController::requestResetCalibration()
    {
        if (m_serial.sendCommand(QStringLiteral("RESET_CAL")))
        {
            emit statusAnnouncement(QStringLiteral("Resetting device calibration to factory defaults."));
        }
    }

    void ManufacturingController::requestSetDeadzone(int axis, int deadzoneCounts)
    {
        if (axis < 0 || axis > 2 || deadzoneCounts < 0)
        {
            return;
        }
        m_serial.sendCommand(QStringLiteral("SET_DZ %1 %2").arg(axis).arg(deadzoneCounts));
    }

    void ManufacturingController::requestSetInvert(int axis, bool invert)
    {
        if (axis < 0 || axis > 2)
        {
            return;
        }
        m_serial.sendCommand(
            QStringLiteral("SET_INV %1 %2").arg(axis).arg(invert ? 1 : 0));
    }

    void ManufacturingController::requestResetManufacturingTest()
    {
        m_model.resetWatermarks();
        emit statusAnnouncement(
            QStringLiteral("Manufacturing test watermarks cleared for next unit under test."));
    }

    bool ManufacturingController::requestButtonlessFirmwareUpdate(const QString& uf2FilePath)
    {
        if (m_serial.isConnected())
        {
            emit statusAnnouncement(
                QStringLiteral("Sending software BOOTSEL command over USB-C and releasing serial port..."));
            m_serial.sendCommand(QStringLiteral("BOOTSEL"));
            m_serial.disconnectPort();
        }

        return m_updater.startUpdate(uf2FilePath);
    }

    void ManufacturingController::onConnectionStateChanged(bool           connected,
                                                           const QString& portName)
    {
        if (connected)
        {
            m_model.resetWatermarks();
            m_serial.sendCommand(QStringLiteral("GET_CAL"));
            emit statusAnnouncement(
                QStringLiteral("Connected to %1 — queried saved calibration.").arg(portName));
        }
        else
        {
            emit statusAnnouncement(QStringLiteral("Serial port disconnected."));
        }
    }
} // namespace p3d::ui
