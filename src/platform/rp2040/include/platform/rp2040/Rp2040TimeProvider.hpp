#pragma once

#include <cstdint>

#include "hal/ITimeProvider.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040TimeProvider final : public hal::ITimeProvider
    {
    public:
        void delayMs(std::uint32_t ms) override;
        void delayUs(std::uint32_t us) override;
        std::uint64_t getTimestampUs() const override;
    };
}
