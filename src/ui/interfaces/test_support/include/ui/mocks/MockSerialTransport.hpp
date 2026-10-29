#pragma once

#include "ui/interfaces/ISerialTransport.hpp"

#include <gmock/gmock.h>

namespace p3d::ui::mocks
{
    class MockSerialTransport : public ISerialTransport
    {
        Q_OBJECT

    public:
        using ISerialTransport::ISerialTransport;

        MOCK_METHOD(bool, connectPort, (const QString& portName, qint32 baudRate), (override));
        MOCK_METHOD(void, disconnectPort, (), (override));
        MOCK_METHOD(bool, isConnected, (), (const, override));
        MOCK_METHOD(QString, currentPortName, (), (const, override));
        MOCK_METHOD(bool, sendCommand, (const QString& commandLine), (override));
        MOCK_METHOD(QStringList, availablePorts, (), (const, override));
    };
} // namespace p3d::ui::mocks
