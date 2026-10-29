#pragma once

#include <QObject>
#include <QString>

namespace p3d::ui
{
    // Event-driven buttonless firmware updater abstraction.
    // Triggers software entry into RP2040 USB-BOOT mode and flashes a UF2 image
    // over USB-C without requiring physical access to the BOOTSEL button.
    class IFirmwareUpdater : public QObject
    {
        Q_OBJECT

    public:
        explicit IFirmwareUpdater(QObject* parent = nullptr)
            : QObject(parent)
        {
        }
        ~IFirmwareUpdater() override = default;

        virtual bool startUpdate(const QString& uf2FilePath) = 0;
        virtual bool isUpdating() const                      = 0;

    signals:
        void updateStarted(const QString& uf2FilePath);
        void progressLog(const QString& logLine);
        void updateFinished(bool success, const QString& summaryMessage);
    };
} // namespace p3d::ui
