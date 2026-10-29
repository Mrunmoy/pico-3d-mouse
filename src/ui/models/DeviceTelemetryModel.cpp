#include "ui/models/DeviceTelemetryModel.hpp"

#include <QRegularExpression>
#include <algorithm>

namespace p3d::ui
{
    DeviceTelemetryModel::DeviceTelemetryModel(const ManufacturingLimits& limits, QObject* parent)
        : QObject(parent)
        , m_limits(limits)
    {
    }

    const AxisLiveState& DeviceTelemetryModel::axisLive(std::size_t axis) const
    {
        return m_live.at(axis < kUiAxisCount ? axis : 0);
    }

    const AxisCalibrationState& DeviceTelemetryModel::axisCalibration(std::size_t axis) const
    {
        return m_cal.at(axis < kUiAxisCount ? axis : 0);
    }

    AxisTestVerdict DeviceTelemetryModel::axisVerdict(std::size_t axis) const
    {
        const std::size_t idx  = axis < kUiAxisCount ? axis : 0;
        const auto&       live = m_live[idx];
        const auto&       cal  = m_cal[idx];

        AxisTestVerdict v{};
        v.hasSamples = live.sampleCount > 0;
        const int inferredCenter =
            std::clamp(static_cast<int>(live.rawCounts) - static_cast<int>(live.deltaCounts),
                       0,
                       4095);
        const std::uint16_t refCenter =
            (cal.centerCounts != 2048) ? cal.centerCounts
                                       : static_cast<std::uint16_t>(inferredCenter);
        v.centerInSpec = v.hasSamples && (refCenter >= m_limits.centerMinCounts) &&
                         (refCenter <= m_limits.centerMaxCounts);
        v.negativeTravelReached =
            v.hasSamples && (live.minObservedRaw <= m_limits.sweepLowThresholdCounts);
        v.positiveTravelReached =
            v.hasSamples && (live.maxObservedRaw >= m_limits.sweepHighThresholdCounts);
        return v;
    }

    bool DeviceTelemetryModel::allAxesPassed() const
    {
        for (std::size_t i = 0; i < kUiAxisCount; ++i)
        {
            if (!axisVerdict(i).passed())
            {
                return false;
            }
        }
        return true;
    }

    const QString& DeviceTelemetryModel::lastCalEvent() const
    {
        return m_lastCalEvent;
    }

    void DeviceTelemetryModel::resetWatermarks()
    {
        for (auto& ax : m_live)
        {
            ax.minObservedRaw = 4095;
            ax.maxObservedRaw = 0;
            ax.sampleCount    = 0;
        }
        m_lastAllPassed = false;
        emit telemetryUpdated();
        emit manufacturingVerdictChanged(false);
    }

    void DeviceTelemetryModel::onSerialLineReceived(const QString& line)
    {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
        {
            return;
        }

        if (trimmed.startsWith(QStringLiteral("@CAL ")))
        {
            if (parseCalibrationLine(trimmed))
            {
                emit calibrationUpdated(m_lastCalEvent);
                const bool nowPassed = allAxesPassed();
                if (nowPassed != m_lastAllPassed)
                {
                    m_lastAllPassed = nowPassed;
                }
                emit manufacturingVerdictChanged(nowPassed);
            }
            return;
        }

        if (trimmed.startsWith('#'))
        {
            emit deviceMessageReceived(trimmed);
            return;
        }

        if (parseTelemetryLine(trimmed))
        {
            emit telemetryUpdated();
            const bool nowPassed = allAxesPassed();
            if (nowPassed != m_lastAllPassed)
            {
                m_lastAllPassed = nowPassed;
            }
            emit manufacturingVerdictChanged(nowPassed);
        }
    }

    bool DeviceTelemetryModel::parseTelemetryLine(const QString& line)
    {
        // Matches 3 blocks of: [raw=<u16> <u32>mV d=<s16> out=<s16>]
        static const QRegularExpression kChannelRegex{
            QStringLiteral(R"(\[raw=\s*(\d+)\s+(\d+)mV\s+d=\s*([+-]?\d+)\s+out=\s*([+-]?\d+)\])")
        };

        auto       it    = kChannelRegex.globalMatch(line);
        std::size_t index = 0;
        while (it.hasNext() && index < kUiAxisCount)
        {
            const QRegularExpressionMatch match = it.next();
            auto&                         live  = m_live[index];

            live.rawCounts    = static_cast<std::uint16_t>(match.captured(1).toUInt());
            live.millivolts   = static_cast<std::uint32_t>(match.captured(2).toUInt());
            live.deltaCounts  = static_cast<std::int16_t>(match.captured(3).toInt());
            live.scaledOutput = static_cast<std::int16_t>(match.captured(4).toInt());

            if (live.rawCounts < live.minObservedRaw)
            {
                live.minObservedRaw = live.rawCounts;
            }
            if (live.rawCounts > live.maxObservedRaw)
            {
                live.maxObservedRaw = live.rawCounts;
            }
            ++live.sampleCount;
            ++index;
        }

        return index == kUiAxisCount;
    }

    bool DeviceTelemetryModel::parseCalibrationLine(const QString& line)
    {
        // Format: @CAL <EVENT> CH0[c=2117 min=0 max=4095 dz=12 inv=0] CH1[...] CH2[...]
        static const QRegularExpression kHeaderRegex{ QStringLiteral(R"(^@CAL\s+(\S+))") };
        static const QRegularExpression kAxisCalRegex{
            QStringLiteral(R"(CH(\d)\[c=(\d+)\s+min=(\d+)\s+max=(\d+)\s+dz=(\d+)\s+inv=([01])\])")
        };

        const QRegularExpressionMatch headerMatch = kHeaderRegex.match(line);
        if (!headerMatch.hasMatch())
        {
            return false;
        }
        m_lastCalEvent = headerMatch.captured(1);

        auto        it      = kAxisCalRegex.globalMatch(line);
        std::size_t matched = 0;
        while (it.hasNext())
        {
            const QRegularExpressionMatch m    = it.next();
            const std::size_t             axis = static_cast<std::size_t>(m.captured(1).toUInt());
            if (axis < kUiAxisCount)
            {
                auto& cal          = m_cal[axis];
                cal.centerCounts   = static_cast<std::uint16_t>(m.captured(2).toUInt());
                cal.minCounts      = static_cast<std::uint16_t>(m.captured(3).toUInt());
                cal.maxCounts      = static_cast<std::uint16_t>(m.captured(4).toUInt());
                cal.deadzoneCounts = static_cast<std::uint16_t>(m.captured(5).toUInt());
                cal.invert         = (m.captured(6).toUInt() != 0U);
                ++matched;
            }
        }
        return matched == kUiAxisCount;
    }
} // namespace p3d::ui
