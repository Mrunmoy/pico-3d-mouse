#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtGlobal>

namespace p3d::ui
{
    // Event-driven serial transport abstraction (Dependency Inversion + Interface Segregation).
    // All I/O notifications are delivered via Qt signals (zero polling).
    class ISerialTransport : public QObject
    {
        Q_OBJECT

    public:
        explicit ISerialTransport(QObject* parent = nullptr)
            : QObject(parent)
        {
        }
        ~ISerialTransport() override = default;

        virtual bool        connectPort(const QString& portName, qint32 baudRate = 115200) = 0;
        virtual void        disconnectPort()                                               = 0;
        virtual bool        isConnected() const                                            = 0;
        virtual QString     currentPortName() const                                        = 0;
        virtual bool        sendCommand(const QString& commandLine)                        = 0;
        virtual QStringList availablePorts() const                                         = 0;

    signals:
        void lineReceived(const QString& line);
        void connectionStateChanged(bool connected, const QString& portName);
        void errorOccurred(const QString& message);
    };
} // namespace p3d::ui
