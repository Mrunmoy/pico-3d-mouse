#include "drivers/OversampledAdcChannel.hpp"

#include <cstdint>

namespace p3d::drivers
{
    OversampledAdcChannel::OversampledAdcChannel(hal::IAdcHardware& adc, const AdcChannelConfig& cfg)
        : m_adc(adc)
        , m_cfg(cfg)
    {
    }

    bool OversampledAdcChannel::init()
    {
        return m_adc.initChannel(m_cfg.channelIndex);
    }

    bool OversampledAdcChannel::read(AdcSample& outSample)
    {
        if (!m_adc.selectChannel(m_cfg.channelIndex))
        {
            return false;
        }

        if (m_cfg.discardSettlingRead)
        {
            std::uint16_t dummy = 0;
            if (!m_adc.readRaw(dummy))
            {
                return false;
            }
        }

        const std::uint8_t n = (m_cfg.oversampleCount == 0U) ? 1U : m_cfg.oversampleCount;
        std::uint32_t sum = 0;

        for (std::uint8_t i = 0; i < n; ++i)
        {
            std::uint16_t sample = 0;
            if (!m_adc.readRaw(sample))
            {
                return false;
            }
            sum += sample;
        }

        const std::uint16_t avgRaw = static_cast<std::uint16_t>(sum / n);
        const std::uint16_t denom  = (m_cfg.maxRawCounts == 0U) ? 1U : m_cfg.maxRawCounts;

        outSample.rawCounts  = avgRaw;
        outSample.millivolts = static_cast<std::uint16_t>(
            (static_cast<std::uint32_t>(avgRaw) * static_cast<std::uint32_t>(m_cfg.vrefMillivolts)) /
            static_cast<std::uint32_t>(denom));

        return true;
    }
}
