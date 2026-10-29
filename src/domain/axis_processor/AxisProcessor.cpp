#include "domain/AxisProcessor.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace p3d::domain
{
    AxisProcessor::AxisProcessor(const AxisConfig& cfg)
        : m_cfg(cfg)
    {
    }

    AxisOutput AxisProcessor::process(std::int16_t rawDelta)
    {
        const std::int32_t dz   = static_cast<std::int32_t>(m_cfg.deadzoneCounts);
        const std::int32_t diff = static_cast<std::int32_t>(rawDelta);

        std::int32_t conditioned = 0;
        if (diff > dz)
        {
            conditioned = diff - dz;
        }
        else if (diff < -dz)
        {
            conditioned = diff + dz;
        }

        if (m_cfg.invert)
        {
            conditioned = -conditioned;
        }

        const float denom =
            (m_cfg.maxDeflectionCounts == 0U) ? 1.0f : static_cast<float>(m_cfg.maxDeflectionCounts);
        const float rawNorm = std::clamp(static_cast<float>(conditioned) / denom, -1.0f, 1.0f);

        const float alpha = std::clamp(m_cfg.emaAlpha, 0.0f, 1.0f);
        if (!m_initialized)
        {
            m_filteredNorm = rawNorm;
            m_initialized  = true;
        }
        else
        {
            m_filteredNorm = (alpha * rawNorm) + ((1.0f - alpha) * m_filteredNorm);
        }

        const float scaledFloat =
            m_filteredNorm * static_cast<float>(m_cfg.outputScaleMax);

        AxisOutput out{};
        out.conditionedDelta = static_cast<std::int16_t>(
            std::clamp<std::int32_t>(conditioned, -32767, 32767));
        out.normalized = m_filteredNorm;
        out.scaled     = static_cast<std::int16_t>(std::lround(scaledFloat));
        return out;
    }

    void AxisProcessor::reset()
    {
        m_filteredNorm = 0.0f;
        m_initialized  = false;
    }
}
