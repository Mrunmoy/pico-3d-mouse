#pragma once

#include <cstdint>

#include "drivers/IAdcChannelReader.hpp"
#include "hal/IAdcHardware.hpp"

namespace p3d::drivers
{
    struct AdcChannelConfig
    {
        std::uint8_t  channelIndex        = 0;
        std::uint8_t  oversampleCount     = 16;
        bool          discardSettlingRead = true;
        std::uint16_t vrefMillivolts      = 3300;
        std::uint16_t maxRawCounts        = 4095;
    };

    class OversampledAdcChannel final : public IAdcChannelReader
    {
    public:
        OversampledAdcChannel(hal::IAdcHardware& adc, const AdcChannelConfig& cfg);

        bool init() override;
        bool read(AdcSample& outSample) override;

        const AdcChannelConfig& config() const { return m_cfg; }

    private:
        hal::IAdcHardware& m_adc;
        AdcChannelConfig   m_cfg;
    };
}
