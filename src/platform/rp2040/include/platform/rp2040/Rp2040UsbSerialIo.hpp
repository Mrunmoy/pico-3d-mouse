#pragma once

#include "hal/ISerialIo.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040UsbSerialIo final : public hal::ISerialIo
    {
    public:
        bool init() override;
        int  readCharNonBlocking() override;
        void writeLine(const char* str) override;
    };
}
