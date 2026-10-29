#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

namespace p3d::ui
{
    // Asynchronous OS process runner abstraction used by PicotoolFirmwareUpdater
    // so process execution can be unit-tested off-target without spawning external binaries.
    class IProcessRunner : public QObject
    {
        Q_OBJECT

    public:
        explicit IProcessRunner(QObject* parent = nullptr)
            : QObject(parent)
        {
        }
        ~IProcessRunner() override = default;

        virtual void startProcess(const QString& program, const QStringList& arguments) = 0;
        virtual bool isRunning() const                                                  = 0;

    signals:
        void outputProduced(const QString& text);
        void processFinished(int exitCode, bool crashed);
        void processFailedToStart(const QString& errorMessage);
    };
} // namespace p3d::ui
