#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// ── Raspberry Pi Pico (RP2040) 3D Mouse Pin Map ───────────────────
//
// Potentiometer wiring:
//   - High side : Pin 36 — 3V3(OUT) (+3.3V)
//   - Low side  : Pin 33 — AGND     (Analog GND)
//   - Wiper 0   : Pin 31 — GP26 / ADC0
//   - Wiper 1   : Pin 32 — GP27 / ADC1
//   - Wiper 2   : Pin 34 — GP28 / ADC2

namespace board::pins
{
    inline constexpr std::size_t kNumAxes = 3;

    // ── Analog potentiometer channels ───────────────────────────
    inline constexpr std::uint32_t kPot0Gpio     = 26;  // Physical Pin 31
    inline constexpr std::uint32_t kPot0AdcInput = 0;   // ADC0

    inline constexpr std::uint32_t kPot1Gpio     = 27;  // Physical Pin 32
    inline constexpr std::uint32_t kPot1AdcInput = 1;   // ADC1

    inline constexpr std::uint32_t kPot2Gpio     = 28;  // Physical Pin 34
    inline constexpr std::uint32_t kPot2AdcInput = 2;   // ADC2

    struct AdcChannelPin
    {
        std::uint32_t gpio;
        std::uint32_t adcInput;
    };

    inline constexpr std::array<AdcChannelPin, kNumAxes> kPotChannels = {{
        { kPot0Gpio, kPot0AdcInput },
        { kPot1Gpio, kPot1AdcInput },
        { kPot2Gpio, kPot2AdcInput },
    }};

    // ── Onboard status LED ──────────────────────────────────────
    inline constexpr std::uint32_t kStatusLedGpio = 25;
}
