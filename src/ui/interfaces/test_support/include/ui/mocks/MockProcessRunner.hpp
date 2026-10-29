#pragma once

#include "ui/interfaces/IProcessRunner.hpp"

#include <gmock/gmock.h>

namespace p3d::ui::mocks
{
    class MockProcessRunner : public IProcessRunner
    {
        Q_OBJECT

    public:
        using IProcessRunner::IProcessRunner;

        MOCK_METHOD(void, startProcess, (const QString& program, const QStringList& arguments), (override));
        MOCK_METHOD(bool, isRunning, (), (const, override));
    };
} // namespace p3d::ui::mocks
