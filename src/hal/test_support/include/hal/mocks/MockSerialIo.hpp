#pragma once

#include "hal/ISerialIo.hpp"
#include <gmock/gmock.h>

namespace p3d::test
{
    class MockSerialIo : public hal::ISerialIo
    {
    public:
        MOCK_METHOD(bool, init, (), (override));
        MOCK_METHOD(int, readCharNonBlocking, (), (override));
        MOCK_METHOD(void, writeLine, (const char* str), (override));
    };
}
