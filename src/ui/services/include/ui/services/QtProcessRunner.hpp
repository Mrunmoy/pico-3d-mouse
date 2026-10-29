#pragma once

#include "ui/interfaces/IProcessRunner.hpp"

#include <QProcess>

namespace p3d::ui
{
    class QtProcessRunner final : public IProcessRunner
    {
        Q_OBJECT

    public:
        explicit QtProcessRunner(QObject* parent = nullptr);
        ~QtProcessRunner() override;

        void startProcess(const QString& program, const QStringList& arguments) override;
        bool isRunning() const override;

    private slots:
        void onReadyReadOutput();
        void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onErrorOccurred(QProcess::ProcessError error);

    private:
        QProcess m_process{};
    };
} // namespace p3d::ui
