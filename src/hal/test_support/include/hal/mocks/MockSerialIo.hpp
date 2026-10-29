#pragma once

#include "hal/ISerialIo.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockSerialIo : public hal::ISerialIo
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(std::optional<char>, readCharNonBlocking, (), (override));
        MOCK_METHOD(void, writeLine, (std::string_view line), (override));
    };
}
