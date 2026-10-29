#pragma once

#include <cstdint>

namespace p3d::drivers
{
    struct AdcSample
    {
        std::uint16_t rawCounts  = 0;
        std::uint16_t millivolts = 0;
    };

    // Narrow interface for reading a conditioned ADC channel (LSP / DIP).
    class IAdcChannelReader
    {
    public:
        virtual ~IAdcChannelReader() = default;

        virtual bool init() = 0;
        virtual bool read(AdcSample& outSample) = 0;
    };
}
