#pragma once

#include <optional>
#include <string_view>

#include "hal/ISerialIo.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040UsbSerialIo final : public hal::ISerialIo
    {
    public:
        bool                init() override;
        std::optional<char> readCharNonBlocking() override;
        void                writeLine(std::string_view line) override;
    };
}
