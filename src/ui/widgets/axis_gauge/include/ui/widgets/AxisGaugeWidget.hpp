#pragma once

#include <QString>
#include <QWidget>
#include <cstdint>

namespace p3d::ui
{
    // Standalone custom QPainter widget rendering a single analog axis channel:
    //   - Raw 12-bit ADC bar (0..4095) and millivolt readout
    //   - Calibrated center marker and symmetric deadzone band
    //   - Observed min/max sweep watermarks
    //   - Conditioned output (-500..+500) and Manufacturing Pass/Pending badge
    // Zero coupling to models, controllers, or transport.
    class AxisGaugeWidget final : public QWidget
    {
        Q_OBJECT

    public:
        explicit AxisGaugeWidget(const QString& axisTitle,
                                 const QString& pinLabel,
                                 QWidget*       parent = nullptr);
        ~AxisGaugeWidget() override = default;

        const QString& axisTitle() const { return m_axisTitle; }
        std::uint16_t  rawCounts() const { return m_rawCounts; }
        std::uint32_t  millivolts() const { return m_millivolts; }
        std::int16_t   scaledOutput() const { return m_scaledOutput; }
        std::uint16_t  centerCounts() const { return m_centerCounts; }
        std::uint16_t  deadzoneCounts() const { return m_deadzoneCounts; }
        std::uint16_t  minObserved() const { return m_minObserved; }
        std::uint16_t  maxObserved() const { return m_maxObserved; }
        bool           manufacturingPassed() const { return m_passed; }

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

    public slots:
        void setReading(std::uint16_t rawCounts,
                        std::uint32_t millivolts,
                        std::int16_t  deltaCounts,
                        std::int16_t  scaledOutput,
                        std::uint16_t minObserved,
                        std::uint16_t maxObserved);

        void setCalibrationMarkers(std::uint16_t centerCounts,
                                   std::uint16_t deadzoneCounts,
                                   bool          inverted);

        void setManufacturingCheckStatus(bool centerOk,
                                         bool negTravelOk,
                                         bool posTravelOk);

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        QString       m_axisTitle{};
        QString       m_pinLabel{};
        std::uint16_t m_rawCounts{ 2048 };
        std::uint32_t m_millivolts{ 1650 };
        std::int16_t  m_deltaCounts{ 0 };
        std::int16_t  m_scaledOutput{ 0 };
        std::uint16_t m_minObserved{ 2048 };
        std::uint16_t m_maxObserved{ 2048 };
        std::uint16_t m_centerCounts{ 2048 };
        std::uint16_t m_deadzoneCounts{ 12 };
        bool          m_inverted{ false };
        bool          m_centerOk{ false };
        bool          m_negTravelOk{ false };
        bool          m_posTravelOk{ false };
        bool          m_passed{ false };
    };
} // namespace p3d::ui
