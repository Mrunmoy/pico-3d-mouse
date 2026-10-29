#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace p3d::drivers
{
    inline constexpr std::size_t kNumPotAxes = 3;

    // Semantic axis indices matching physical potentiometer wiring:
    //   - Forward / Backward wiper -> ADC0 (GP26, Pin 31)
    //   - Left / Right wiper       -> ADC1 (GP27, Pin 32)
    //   - Rotation (Twist) wiper   -> ADC2 (GP28, Pin 34)
    enum class AxisId : std::size_t
    {
        ForwardBackward = 0,
        LeftRight       = 1,
        Rotation        = 2
    };

    struct PotentiometerSnapshot
    {
        std::array<std::uint16_t, kNumPotAxes> rawCounts{};
        std::array<std::uint16_t, kNumPotAxes> millivolts{};
        std::array<std::int16_t,  kNumPotAxes> rawDelta{};
    };

    class IPotentiometerSensor
    {
    public:
        virtual ~IPotentiometerSensor() = default;

        virtual bool init() = 0;
        virtual bool calibrateCenter(std::uint16_t numSweeps) = 0;
        virtual bool readSnapshot(PotentiometerSnapshot& outSnapshot) = 0;
        virtual const std::array<std::uint16_t, kNumPotAxes>& centerCounts() const = 0;
    };
}
