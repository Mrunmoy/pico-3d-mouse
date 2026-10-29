#include "ui/services/QtProcessRunner.hpp"

namespace p3d::ui
{
    QtProcessRunner::QtProcessRunner(QObject* parent)
        : IProcessRunner(parent)
    {
        m_process.setProcessChannelMode(QProcess::MergedChannels);
        connect(&m_process,
                &QProcess::readyReadStandardOutput,
                this,
                &QtProcessRunner::onReadyReadOutput);
        connect(&m_process, &QProcess::finished, this, &QtProcessRunner::onFinished);
        connect(&m_process, &QProcess::errorOccurred, this, &QtProcessRunner::onErrorOccurred);
    }

    QtProcessRunner::~QtProcessRunner()
    {
        if (m_process.state() != QProcess::NotRunning)
        {
            m_process.kill();
            m_process.waitForFinished(1000);
        }
    }

    void QtProcessRunner::startProcess(const QString& program, const QStringList& arguments)
    {
        m_process.start(program, arguments);
    }

    bool QtProcessRunner::isRunning() const
    {
        return m_process.state() != QProcess::NotRunning;
    }

    void QtProcessRunner::onReadyReadOutput()
    {
        const QString text = QString::fromUtf8(m_process.readAllStandardOutput());
        if (!text.isEmpty())
        {
            emit outputProduced(text);
        }
    }

    void QtProcessRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
    {
        onReadyReadOutput();
        emit processFinished(exitCode, exitStatus == QProcess::CrashExit);
    }

    void QtProcessRunner::onErrorOccurred(QProcess::ProcessError error)
    {
        if (error == QProcess::FailedToStart)
        {
            emit processFailedToStart(m_process.errorString());
        }
    }
} // namespace p3d::ui
