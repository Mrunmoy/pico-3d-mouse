#pragma once

#include <cstdint>

namespace p3d::hal
{
    // Pure hardware abstraction for a multiplexed SAR ADC block.
    // Segregated from pin-mux details: callers address logical ADC inputs (0..N-1).
    class IAdcHardware
    {
    public:
        virtual ~IAdcHardware() = default;

        // Initializes the ADC peripheral block.
        virtual bool init() = 0;

        // Configures the GPIO associated with the given ADC channel for high-Z analog input.
        virtual bool initChannel(std::uint8_t channel) = 0;

        // Selects the active analog multiplexer input channel.
        virtual bool selectChannel(std::uint8_t channel) = 0;

        // Performs a single blocking ADC conversion and stores the raw 12-bit count in `outCounts`.
        virtual bool readRaw(std::uint16_t& outCounts) = 0;
    };
}
