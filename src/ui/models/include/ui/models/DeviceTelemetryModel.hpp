#pragma once

#include <QObject>
#include <QString>
#include <array>
#include <cstddef>
#include <cstdint>

namespace p3d::ui
{
    inline constexpr std::size_t kUiAxisCount = 3;

    struct AxisLiveState
    {
        std::uint16_t rawCounts{ 2048 };
        std::uint32_t millivolts{ 1650 };
        std::int16_t  deltaCounts{ 0 };
        std::int16_t  scaledOutput{ 0 };
        std::uint16_t minObservedRaw{ 4095 };
        std::uint16_t maxObservedRaw{ 0 };
        std::uint32_t sampleCount{ 0 };
    };

    struct AxisCalibrationState
    {
        std::uint16_t centerCounts{ 2048 };
        std::uint16_t minCounts{ 0 };
        std::uint16_t maxCounts{ 4095 };
        std::uint16_t deadzoneCounts{ 12 };
        bool          invert{ false };
    };

    struct ManufacturingLimits
    {
        std::uint16_t centerMinCounts{ 1400 };
        std::uint16_t centerMaxCounts{ 2700 };
        std::uint16_t sweepLowThresholdCounts{ 900 };
        std::uint16_t sweepHighThresholdCounts{ 3200 };
    };

    struct AxisTestVerdict
    {
        bool hasSamples{ false };
        bool centerInSpec{ false };
        bool negativeTravelReached{ false };
        bool positiveTravelReached{ false };

        constexpr bool passed() const
        {
            return hasSamples && centerInSpec && negativeTravelReached && positiveTravelReached;
        }
    };

    class DeviceTelemetryModel final : public QObject
    {
        Q_OBJECT

    public:
        explicit DeviceTelemetryModel(const ManufacturingLimits& limits = ManufacturingLimits{},
                                      QObject*                   parent = nullptr);
        ~DeviceTelemetryModel() override = default;

        const AxisLiveState&        axisLive(std::size_t axis) const;
        const AxisCalibrationState& axisCalibration(std::size_t axis) const;
        AxisTestVerdict             axisVerdict(std::size_t axis) const;
        bool                        allAxesPassed() const;
        const QString&              lastCalEvent() const;

    public slots:
        void onSerialLineReceived(const QString& line);
        void resetWatermarks();

    signals:
        void telemetryUpdated();
        void calibrationUpdated(const QString& eventName);
        void manufacturingVerdictChanged(bool allPassed);
        void deviceMessageReceived(const QString& message);

    private:
        bool parseTelemetryLine(const QString& line);
        bool parseCalibrationLine(const QString& line);

        ManufacturingLimits                            m_limits{};
        std::array<AxisLiveState, kUiAxisCount>        m_live{};
        std::array<AxisCalibrationState, kUiAxisCount> m_cal{};
        QString                                        m_lastCalEvent{};
        bool                                           m_lastAllPassed{ false };
    };
} // namespace p3d::ui
