#include "ui/services/PicotoolFirmwareUpdater.hpp"

#include <QFileInfo>

namespace p3d::ui
{
    PicotoolFirmwareUpdater::PicotoolFirmwareUpdater(IProcessRunner& processRunner,
                                                     bool            verifyFileExists,
                                                     QObject*        parent)
        : IFirmwareUpdater(parent)
        , m_runner(processRunner)
        , m_verifyFileExists(verifyFileExists)
    {
        connect(&m_runner,
                &IProcessRunner::outputProduced,
                this,
                &PicotoolFirmwareUpdater::onRunnerOutput);
        connect(&m_runner,
                &IProcessRunner::processFinished,
                this,
                &PicotoolFirmwareUpdater::onRunnerFinished);
        connect(&m_runner,
                &IProcessRunner::processFailedToStart,
                this,
                &PicotoolFirmwareUpdater::onRunnerFailedToStart);
    }

    bool PicotoolFirmwareUpdater::startUpdate(const QString& uf2FilePath)
    {
        if (m_updating || m_runner.isRunning())
        {
            emit progressLog(QStringLiteral("Firmware update is already in progress."));
            return false;
        }

        const QString trimmed = uf2FilePath.trimmed();
        if (trimmed.isEmpty() || !trimmed.endsWith(QStringLiteral(".uf2"), Qt::CaseInsensitive))
        {
            const QString msg = QStringLiteral("Invalid firmware image: must be a .uf2 file.");
            emit progressLog(msg);
            emit updateFinished(false, msg);
            return false;
        }

        if (m_verifyFileExists && !QFileInfo::exists(trimmed))
        {
            const QString msg = QStringLiteral("Firmware file not found: %1").arg(trimmed);
            emit progressLog(msg);
            emit updateFinished(false, msg);
            return false;
        }

        m_updating   = true;
        m_currentUf2 = trimmed;

        emit updateStarted(m_currentUf2);
        emit progressLog(
            QStringLiteral("Starting buttonless USB firmware update with image: %1")
                .arg(m_currentUf2));
        emit progressLog(
            QStringLiteral("Executing: picotool load -f -v -x %1 (no physical BOOTSEL press needed)")
                .arg(m_currentUf2));

        // -f : Force RP2040 into BOOTSEL mode via USB reset interface if running application
        // -v : Verify flash contents after programming
        // -x : Reboot and execute newly flashed firmware immediately
        m_runner.startProcess(
            QStringLiteral("picotool"),
            QStringList{ QStringLiteral("load"), QStringLiteral("-f"), QStringLiteral("-v"), QStringLiteral("-x"), m_currentUf2 });
        return true;
    }

    bool PicotoolFirmwareUpdater::isUpdating() const
    {
        return m_updating;
    }

    void PicotoolFirmwareUpdater::onRunnerOutput(const QString& text)
    {
        const QStringList lines = text.split('\n', Qt::SkipEmptyParts);
        for (const QString& rawLine : lines)
        {
            const QString cleaned = rawLine.trimmed();
            if (!cleaned.isEmpty())
            {
                emit progressLog(cleaned);
            }
        }
    }

    void PicotoolFirmwareUpdater::onRunnerFinished(int exitCode, bool crashed)
    {
        m_updating = false;
        if (!crashed && exitCode == 0)
        {
            const QString msg =
                QStringLiteral("Firmware update complete (%1). Device rebooted into application.")
                    .arg(QFileInfo(m_currentUf2).fileName());
            emit progressLog(msg);
            emit updateFinished(true, msg);
        }
        else
        {
            const QString msg =
                QStringLiteral("Firmware update failed (picotool exit code %1).").arg(exitCode);
            emit progressLog(msg);
            emit updateFinished(false, msg);
        }
    }

    void PicotoolFirmwareUpdater::onRunnerFailedToStart(const QString& errorMessage)
    {
        m_updating = false;
        const QString msg =
            QStringLiteral("Failed to launch picotool: %1").arg(errorMessage);
        emit progressLog(msg);
        emit updateFinished(false, msg);
    }
} // namespace p3d::ui
