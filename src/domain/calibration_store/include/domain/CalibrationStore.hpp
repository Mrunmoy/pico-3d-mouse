#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/INvmStorage.hpp"

namespace p3d::domain
{
    inline constexpr std::size_t kCalibrationAxes = 3;

    struct CalibrationData
    {
        std::array<std::uint16_t, kCalibrationAxes> centerCounts{ 2048, 2048, 2048 };
        std::array<std::uint16_t, kCalibrationAxes> minCounts{ 0, 0, 0 };
        std::array<std::uint16_t, kCalibrationAxes> maxCounts{ 4095, 4095, 4095 };
        std::array<std::uint16_t, kCalibrationAxes> deadzoneCounts{ 12, 12, 12 };
        std::array<bool,          kCalibrationAxes> invert{ false, false, false };
    };

    class ICalibrationStore
    {
    public:
        virtual ~ICalibrationStore() = default;

        virtual bool load(CalibrationData& outData) const = 0;
        virtual bool save(const CalibrationData& data) = 0;
    };

    class CalibrationStore final : public ICalibrationStore
    {
    public:
        static constexpr std::size_t kSerializedSize = 36;

        explicit CalibrationStore(hal::INvmStorage& nvm);

        bool load(CalibrationData& outData) const override;
        bool save(const CalibrationData& data) override;

        static bool isValidData(const CalibrationData& data);

    private:
        hal::INvmStorage& m_nvm;
    };
}
