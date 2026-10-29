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

namespace p3d::platform::rp2040::pins
{
    inline constexpr std::size_t kNumAdcChannels = 3;

    inline constexpr std::uint32_t kPot0Gpio = 26;  // Physical Pin 31 (ADC0)
    inline constexpr std::uint32_t kPot1Gpio = 27;  // Physical Pin 32 (ADC1)
    inline constexpr std::uint32_t kPot2Gpio = 28;  // Physical Pin 34 (ADC2)

    inline constexpr std::array<std::uint32_t, kNumAdcChannels> kAdcChannelGpios = {{
        kPot0Gpio,
        kPot1Gpio,
        kPot2Gpio,
    }};

    inline constexpr std::uint32_t kStatusLedGpio = 25;
}
