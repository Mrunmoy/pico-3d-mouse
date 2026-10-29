#pragma once

#include <array>
#include <cstdint>

#include "drivers/IAdcChannelReader.hpp"
#include "drivers/IPotentiometerSensor.hpp"

namespace p3d::drivers
{
    class PotentiometerArray final : public IPotentiometerSensor
    {
    public:
        PotentiometerArray(IAdcChannelReader& ch0,
                           IAdcChannelReader& ch1,
                           IAdcChannelReader& ch2);

        bool init() override;
        bool calibrateCenter(std::uint16_t numSweeps = 64) override;
        bool readSnapshot(PotentiometerSnapshot& outSnapshot) override;

        const std::array<std::uint16_t, kNumPotAxes>& centerCounts() const override
        {
            return m_center;
        }

    private:
        std::array<IAdcChannelReader*, kNumPotAxes> m_channels;
        std::array<std::uint16_t,      kNumPotAxes> m_center{ 2048, 2048, 2048 };
    };
}
