#pragma once

#include "ui/interfaces/IFirmwareUpdater.hpp"
#include "ui/interfaces/IProcessRunner.hpp"

namespace p3d::ui
{
    class PicotoolFirmwareUpdater final : public IFirmwareUpdater
    {
        Q_OBJECT

    public:
        explicit PicotoolFirmwareUpdater(IProcessRunner& processRunner,
                                         bool            verifyFileExists = true,
                                         QObject*        parent           = nullptr);
        ~PicotoolFirmwareUpdater() override = default;

        bool startUpdate(const QString& uf2FilePath) override;
        bool isUpdating() const override;

    private slots:
        void onRunnerOutput(const QString& text);
        void onRunnerFinished(int exitCode, bool crashed);
        void onRunnerFailedToStart(const QString& errorMessage);

    private:
        IProcessRunner& m_runner;
        bool            m_verifyFileExists{ true };
        bool            m_updating{ false };
        QString         m_currentUf2{};
    };
} // namespace p3d::ui
