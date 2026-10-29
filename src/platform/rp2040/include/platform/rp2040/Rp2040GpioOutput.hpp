#pragma once

#include <cstdint>

#include "hal/IGpioOutput.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040GpioOutput final : public hal::IGpioOutput
    {
    public:
        explicit Rp2040GpioOutput(std::uint32_t gpioPin);

        bool init() override;
        bool write(bool level) override;

    private:
        std::uint32_t m_gpioPin;
    };
}
