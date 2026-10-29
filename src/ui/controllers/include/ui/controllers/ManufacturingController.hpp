#pragma once

#include "ui/interfaces/IFirmwareUpdater.hpp"
#include "ui/interfaces/ISerialTransport.hpp"
#include "ui/models/DeviceTelemetryModel.hpp"

#include <QObject>
#include <QString>

namespace p3d::ui
{
    // Pure MVC Controller coordinating ISerialTransport, IFirmwareUpdater, and DeviceTelemetryModel
    // purely via Qt Signals & Slots (zero polling).
    class ManufacturingController final : public QObject
    {
        Q_OBJECT

    public:
        ManufacturingController(ISerialTransport&     serialTransport,
                                IFirmwareUpdater&     firmwareUpdater,
                                DeviceTelemetryModel& telemetryModel,
                                QObject*              parent = nullptr);
        ~ManufacturingController() override = default;

    public slots:
        bool requestConnect(const QString& portName, qint32 baudRate = 115200);
        void requestDisconnect();
        void requestZeroCenter();
        void requestMinMaxSweepStart();
        void requestMinMaxSweepEnd();
        void requestSaveCalibration();
        void requestLoadCalibration();
        void requestResetCalibration();
        void requestSetDeadzone(int axis, int deadzoneCounts);
        void requestSetInvert(int axis, bool invert);
        void requestResetManufacturingTest();
        bool requestButtonlessFirmwareUpdate(const QString& uf2FilePath);

    signals:
        void statusAnnouncement(const QString& message);

    private slots:
        void onConnectionStateChanged(bool connected, const QString& portName);

    private:
        ISerialTransport&     m_serial;
        IFirmwareUpdater&     m_updater;
        DeviceTelemetryModel& m_model;
    };
} // namespace p3d::ui
