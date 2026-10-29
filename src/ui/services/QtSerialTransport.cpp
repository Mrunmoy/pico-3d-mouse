#include "ui/services/QtSerialTransport.hpp"

#include <QSerialPortInfo>

namespace p3d::ui
{
    QtSerialTransport::QtSerialTransport(QObject* parent)
        : ISerialTransport(parent)
    {
        connect(&m_serial, &QSerialPort::readyRead, this, &QtSerialTransport::onReadyRead);
        connect(&m_serial,
                &QSerialPort::errorOccurred,
                this,
                &QtSerialTransport::onSerialError);
    }

    QtSerialTransport::~QtSerialTransport()
    {
        if (m_serial.isOpen())
        {
            m_serial.close();
        }
    }

    bool QtSerialTransport::connectPort(const QString& portName, qint32 baudRate)
    {
        if (m_serial.isOpen())
        {
            m_serial.close();
        }
        m_rxBuffer.clear();

        m_serial.setPortName(portName);
        m_serial.setBaudRate(baudRate);
        m_serial.setDataBits(QSerialPort::Data8);
        m_serial.setParity(QSerialPort::NoParity);
        m_serial.setStopBits(QSerialPort::OneStop);
        m_serial.setFlowControl(QSerialPort::NoFlowControl);

        if (!m_serial.open(QIODevice::ReadWrite))
        {
            emit errorOccurred(
                QStringLiteral("Failed to open %1: %2").arg(portName, m_serial.errorString()));
            emit connectionStateChanged(false, QString{});
            return false;
        }

        // Assert DTR/RTS so USB-CDC stdio on RP2040 knows host terminal is connected.
        m_serial.setDataTerminalReady(true);
        m_serial.setRequestToSend(true);

        emit connectionStateChanged(true, m_serial.portName());
        return true;
    }

    void QtSerialTransport::disconnectPort()
    {
        if (m_serial.isOpen())
        {
            m_serial.close();
        }
        m_rxBuffer.clear();
        emit connectionStateChanged(false, QString{});
    }

    bool QtSerialTransport::isConnected() const
    {
        return m_serial.isOpen();
    }

    QString QtSerialTransport::currentPortName() const
    {
        return m_serial.isOpen() ? m_serial.portName() : QString{};
    }

    bool QtSerialTransport::sendCommand(const QString& commandLine)
    {
        if (!m_serial.isOpen())
        {
            emit errorOccurred(QStringLiteral("Cannot send command: serial port is not open"));
            return false;
        }

        QByteArray payload = commandLine.toUtf8();
        if (!payload.endsWith('\n'))
        {
            payload.append('\n');
        }

        const qint64 written = m_serial.write(payload);
        m_serial.flush();
        return written == payload.size();
    }

    QStringList QtSerialTransport::availablePorts() const
    {
        QStringList ports;
        const auto  infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo& info : infos)
        {
            // Prioritize RP2040 / USB ACM ports at the top of the list
            if (info.vendorIdentifier() == 0x2E8A || info.portName().startsWith(QStringLiteral("ttyACM")) ||
                info.portName().startsWith(QStringLiteral("cu.usbmodem")))
            {
                ports.prepend(info.systemLocation());
            }
            else
            {
                ports.append(info.systemLocation());
            }
        }
        return ports;
    }

    void QtSerialTransport::ingestRawBytes(const QByteArray& chunk)
    {
        m_rxBuffer.append(chunk);

        while (true)
        {
            const qsizetype newlineIdx = m_rxBuffer.indexOf('\n');
            if (newlineIdx < 0)
            {
                break;
            }

            QByteArray rawLine = m_rxBuffer.left(newlineIdx);
            m_rxBuffer.remove(0, newlineIdx + 1);

            while (rawLine.endsWith('\r'))
            {
                rawLine.chop(1);
            }

            const QString line = QString::fromUtf8(rawLine).trimmed();
            if (!line.isEmpty())
            {
                emit lineReceived(line);
            }
        }
    }

    void QtSerialTransport::onReadyRead()
    {
        ingestRawBytes(m_serial.readAll());
    }

    void QtSerialTransport::onSerialError(QSerialPort::SerialPortError error)
    {
        if (error == QSerialPort::NoError)
        {
            return;
        }

        if (error == QSerialPort::ResourceError || error == QSerialPort::PermissionError)
        {
            const QString msg = m_serial.errorString();
            m_serial.close();
            emit errorOccurred(msg);
            emit connectionStateChanged(false, QString{});
        }
    }
} // namespace p3d::ui
