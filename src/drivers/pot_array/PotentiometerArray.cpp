#include "drivers/PotentiometerArray.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace p3d::drivers
{
    PotentiometerArray::PotentiometerArray(IAdcChannelReader& ch0,
                                           IAdcChannelReader& ch1,
                                           IAdcChannelReader& ch2)
        : m_channels{ &ch0, &ch1, &ch2 }
    {
    }

    bool PotentiometerArray::init()
    {
        for (auto* ch : m_channels)
        {
            if (!ch->init())
            {
                return false;
            }
        }
        return true;
    }

    bool PotentiometerArray::calibrateCenter(std::uint16_t numSweeps)
    {
        const std::uint16_t sweeps = (numSweeps == 0U) ? 1U : numSweeps;
        std::array<std::uint32_t, kNumPotAxes> sum{ 0, 0, 0 };

        for (std::uint16_t s = 0; s < sweeps; ++s)
        {
            for (std::size_t i = 0; i < kNumPotAxes; ++i)
            {
                AdcSample sample{};
                if (!m_channels[i]->read(sample))
                {
                    return false;
                }
                sum[i] += sample.rawCounts;
            }
        }

        for (std::size_t i = 0; i < kNumPotAxes; ++i)
        {
            m_center[i] = static_cast<std::uint16_t>(sum[i] / sweeps);
        }

        return true;
    }

    bool PotentiometerArray::readSnapshot(PotentiometerSnapshot& outSnapshot)
    {
        for (std::size_t i = 0; i < kNumPotAxes; ++i)
        {
            AdcSample sample{};
            if (!m_channels[i]->read(sample))
            {
                return false;
            }

            outSnapshot.rawCounts[i]  = sample.rawCounts;
            outSnapshot.millivolts[i] = sample.millivolts;
            outSnapshot.rawDelta[i]   = static_cast<std::int16_t>(
                static_cast<std::int32_t>(sample.rawCounts) -
                static_cast<std::int32_t>(m_center[i]));
        }

        return true;
    }
}
