#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "board/pins.hpp"

namespace drivers::pots
{
    inline constexpr std::size_t kNumChannels = board::pins::kNumAxes;

    struct Config
    {
        std::array<board::pins::AdcChannelPin, kNumChannels> channels = board::pins::kPotChannels;
        std::uint8_t  oversampleCount = 16;   // Samples averaged per channel read
        std::uint16_t deadzoneCounts  = 12;   // Deadband around resting center (in 12-bit counts)
    };

    struct Sample
    {
        std::array<std::uint16_t, kNumChannels> raw{};         // 0 .. 4095
        std::array<std::uint16_t, kNumChannels> millivolts{};  // 0 .. 3300 mV
        std::array<std::int16_t,  kNumChannels> delta{};       // Center-relative counts (deadzone applied)
        std::array<float,         kNumChannels> normalized{};  // [-1.0f, +1.0f]
    };

    class Reader
    {
    public:
        explicit Reader(const Config& cfg = Config{});

        // Initializes the RP2040 ADC block and configures GP26..GP28 for high-Z analog input.
        void init();

        // Samples the resting position of all 3 potentiometers and stores the zero offsets.
        void calibrateCenter(std::uint16_t numSamples = 64);

        // Reads all 3 channels with mux settling + oversampling.
        Sample read() const;

        // Returns the currently calibrated resting center (in 12-bit ADC counts).
        const std::array<std::uint16_t, kNumChannels>& centerCounts() const { return m_center; }

    private:
        std::uint16_t readChannelRaw(std::uint32_t adcInput, std::uint8_t samples) const;

        Config                                  m_cfg;
        std::array<std::uint16_t, kNumChannels> m_center{ 2048, 2048, 2048 };
    };
}
