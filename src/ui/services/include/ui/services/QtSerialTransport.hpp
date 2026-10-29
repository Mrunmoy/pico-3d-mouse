#pragma once

#include "ui/interfaces/ISerialTransport.hpp"

#include <QByteArray>
#include <QSerialPort>

namespace p3d::ui
{
    class QtSerialTransport final : public ISerialTransport
    {
        Q_OBJECT

    public:
        explicit QtSerialTransport(QObject* parent = nullptr);
        ~QtSerialTransport() override;

        bool        connectPort(const QString& portName, qint32 baudRate = 115200) override;
        void        disconnectPort() override;
        bool        isConnected() const override;
        QString     currentPortName() const override;
        bool        sendCommand(const QString& commandLine) override;
        QStringList availablePorts() const override;

        // Exposed for deterministic off-target unit testing of chunked line framing
        // without requiring a physical tty device.
        void ingestRawBytes(const QByteArray& chunk);

    private slots:
        void onReadyRead();
        void onSerialError(QSerialPort::SerialPortError error);

    private:
        QSerialPort m_serial{};
        QByteArray  m_rxBuffer{};
    };
} // namespace p3d::ui
