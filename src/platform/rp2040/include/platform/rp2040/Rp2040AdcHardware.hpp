#pragma once

#include <array>
#include <cstdint>

#include "hal/IAdcHardware.hpp"
#include "platform/rp2040/BoardPins.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040AdcHardware final : public hal::IAdcHardware
    {
    public:
        explicit Rp2040AdcHardware(
            const std::array<std::uint32_t, pins::kNumAdcChannels>& channelGpios =
                pins::kAdcChannelGpios);

        bool init() override;
        bool initChannel(std::uint8_t channel) override;
        bool selectChannel(std::uint8_t channel) override;
        bool readRaw(std::uint16_t& outCounts) override;

    private:
        std::array<std::uint32_t, pins::kNumAdcChannels> m_channelGpios;
        bool                                             m_initialized = false;
    };
}
