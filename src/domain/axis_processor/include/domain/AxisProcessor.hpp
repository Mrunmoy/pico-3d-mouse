#pragma once

#include <cstdint>

namespace p3d::domain
{
    struct AxisConfig
    {
        std::uint16_t deadzoneCounts      = 12;     // Symmetric deadband around center
        std::uint16_t maxDeflectionCounts = 2035;   // Full-scale travel past deadzone
        std::int16_t  outputScaleMax      = 500;    // Full-scale integer output (e.g. HID units)
        float         emaAlpha            = 1.0f;   // 1.0 = no smoothing; (0, 1) = low-pass EMA
        bool          invert              = false;  // Invert axis direction
    };

    struct AxisOutput
    {
        std::int16_t conditionedDelta = 0;     // Deadzone-stripped, polarity-applied delta counts
        float        normalized       = 0.0f;  // Filtered value in [-1.0f, +1.0f]
        std::int16_t scaled           = 0;     // Filtered integer in [-outputScaleMax, +outputScaleMax]
    };

    class AxisProcessor
    {
    public:
        explicit AxisProcessor(const AxisConfig& cfg = AxisConfig{});

        // Processes raw signed delta (rawCounts - centerCounts) into conditioned axis output.
        AxisOutput process(std::int16_t rawDelta);

        // Resets internal filter state to zero.
        void reset();

        const AxisConfig& config() const { return m_cfg; }
        void setConfig(const AxisConfig& cfg) { m_cfg = cfg; }

    private:
        AxisConfig m_cfg;
        float      m_filteredNorm = 0.0f;
        bool       m_initialized  = false;
    };
}
