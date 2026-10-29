#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/INvmStorage.hpp"

namespace p3d::platform::rp2040
{
    // Stores persistent calibration in the final 4 KB sector of the RP2040 onboard QSPI flash.
    class Rp2040FlashStorage final : public hal::INvmStorage
    {
    public:
        bool        read(std::uint8_t* outData, std::size_t length) const override;
        bool        write(const std::uint8_t* data, std::size_t length) override;
        std::size_t capacity() const override;
    };
}
