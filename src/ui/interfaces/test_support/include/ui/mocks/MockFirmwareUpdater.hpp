#pragma once

#include "ui/interfaces/IFirmwareUpdater.hpp"

#include <gmock/gmock.h>

namespace p3d::ui::mocks
{
    class MockFirmwareUpdater : public IFirmwareUpdater
    {
        Q_OBJECT

    public:
        using IFirmwareUpdater::IFirmwareUpdater;

        MOCK_METHOD(bool, startUpdate, (const QString& uf2FilePath), (override));
        MOCK_METHOD(bool, isUpdating, (), (const, override));
    };
} // namespace p3d::ui::mocks
