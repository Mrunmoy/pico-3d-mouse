#include "ui/widgets/AxisGaugeWidget.hpp"

#include <QPainter>
#include <QPen>
#include <QRectF>
#include <algorithm>

namespace p3d::ui
{
    AxisGaugeWidget::AxisGaugeWidget(const QString& axisTitle,
                                     const QString& pinLabel,
                                     QWidget*       parent)
        : QWidget(parent)
        , m_axisTitle(axisTitle)
        , m_pinLabel(pinLabel)
    {
        setMinimumHeight(88);
    }

    QSize AxisGaugeWidget::minimumSizeHint() const
    {
        return QSize{ 320, 88 };
    }

    QSize AxisGaugeWidget::sizeHint() const
    {
        return QSize{ 440, 96 };
    }

    void AxisGaugeWidget::setReading(std::uint16_t rawCounts,
                                     std::uint32_t millivolts,
                                     std::int16_t  deltaCounts,
                                     std::int16_t  scaledOutput,
                                     std::uint16_t minObserved,
                                     std::uint16_t maxObserved)
    {
        m_rawCounts    = std::min<std::uint16_t>(rawCounts, 4095U);
        m_millivolts   = millivolts;
        m_deltaCounts  = deltaCounts;
        m_scaledOutput = scaledOutput;
        m_minObserved  = std::min<std::uint16_t>(minObserved, 4095U);
        m_maxObserved  = std::min<std::uint16_t>(maxObserved, 4095U);
        update();
    }

    void AxisGaugeWidget::setCalibrationMarkers(std::uint16_t centerCounts,
                                                std::uint16_t deadzoneCounts,
                                                bool          inverted)
    {
        m_centerCounts   = std::min<std::uint16_t>(centerCounts, 4095U);
        m_deadzoneCounts = deadzoneCounts;
        m_inverted       = inverted;
        update();
    }

    void AxisGaugeWidget::setManufacturingCheckStatus(bool centerOk,
                                                      bool negTravelOk,
                                                      bool posTravelOk)
    {
        m_centerOk    = centerOk;
        m_negTravelOk = negTravelOk;
        m_posTravelOk = posTravelOk;
        m_passed      = centerOk && negTravelOk && posTravelOk;
        update();
    }

    void AxisGaugeWidget::paintEvent(QPaintEvent* /*event*/)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF card = rect().adjusted(2.0, 2.0, -2.0, -2.0);
        painter.setPen(QPen(m_passed ? QColor(16, 185, 129) : QColor(48, 54, 61), 1.5));
        painter.setBrush(QColor(18, 22, 28));
        painter.drawRoundedRect(card, 8.0, 8.0);

        // Header line: Title + Pin on left, Pass/Pending badge on right
        QFont titleFont = font();
        titleFont.setPointSize(9);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.setPen(QColor(230, 237, 243));
        painter.drawText(
            card.adjusted(10, 6, -110, -40),
            Qt::AlignLeft | Qt::AlignTop,
            QStringLiteral("%1 (%2)%3")
                .arg(m_axisTitle, m_pinLabel, m_inverted ? QStringLiteral(" [INV]") : QString{}));

        const QRectF badgeRect(card.right() - 104.0, card.top() + 6.0, 94.0, 20.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_passed ? QColor(6, 95, 70) : QColor(55, 65, 81));
        painter.drawRoundedRect(badgeRect, 4.0, 4.0);
        painter.setPen(m_passed ? QColor(167, 243, 208) : QColor(209, 213, 219));
        painter.drawText(badgeRect,
                         Qt::AlignCenter,
                         m_passed ? QStringLiteral("MFG PASS") : QStringLiteral("SWEEP TEST"));

        // Bar track (0..4095)
        const QRectF barRect(card.left() + 10.0,
                             card.top() + 32.0,
                             card.width() - 20.0,
                             18.0);
        painter.setPen(QPen(QColor(48, 54, 61), 1.0));
        painter.setBrush(QColor(13, 17, 23));
        painter.drawRoundedRect(barRect, 4.0, 4.0);

        auto countsToX = [&](std::uint16_t counts) -> double {
            const double frac = std::clamp(static_cast<double>(counts) / 4095.0, 0.0, 1.0);
            return barRect.left() + frac * barRect.width();
        };

        // Observed min..max watermark span
        if (m_maxObserved >= m_minObserved)
        {
            const double xMin = countsToX(m_minObserved);
            const double xMax = countsToX(m_maxObserved);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(56, 189, 248, 55));
            painter.drawRect(QRectF(xMin, barRect.top() + 2.0, std::max(2.0, xMax - xMin), barRect.height() - 4.0));
        }

        // Deadzone band around center
        const std::uint16_t dzLow =
            (m_centerCounts > m_deadzoneCounts)
                ? static_cast<std::uint16_t>(m_centerCounts - m_deadzoneCounts)
                : 0U;
        const std::uint16_t dzHigh =
            std::min<std::uint16_t>(4095U, static_cast<std::uint16_t>(m_centerCounts + m_deadzoneCounts));
        const double xDzLow  = countsToX(dzLow);
        const double xDzHigh = countsToX(dzHigh);
        painter.setBrush(QColor(245, 158, 11, 90));
        painter.drawRect(QRectF(xDzLow, barRect.top() + 1.0, std::max(2.0, xDzHigh - xDzLow), barRect.height() - 2.0));

        // Active deflection bar from center to rawCounts
        const double xCenter = countsToX(m_centerCounts);
        const double xRaw    = countsToX(m_rawCounts);
        painter.setBrush(QColor(16, 185, 129));
        painter.drawRect(QRectF(std::min(xCenter, xRaw),
                                barRect.top() + 3.0,
                                std::max(3.0, std::abs(xRaw - xCenter)),
                                barRect.height() - 6.0));

        // Center tick line
        painter.setPen(QPen(QColor(251, 191, 36), 2.0));
        painter.drawLine(QPointF(xCenter, barRect.top() - 2.0),
                         QPointF(xCenter, barRect.bottom() + 2.0));

        // Telemetry & check status footer line
        QFont metricsFont = font();
        metricsFont.setPointSize(8);
        metricsFont.setBold(false);
        painter.setFont(metricsFont);
        painter.setPen(QColor(139, 148, 158));

        const QString metricsText =
            QStringLiteral("Raw: %1 (%2 mV)   Δ: %3   Out: %4   Range:[%5..%6]   Checks:[C:%7 Min:%8 Max:%9]")
                .arg(m_rawCounts)
                .arg(m_millivolts)
                .arg(m_deltaCounts)
                .arg(m_scaledOutput)
                .arg(m_minObserved)
                .arg(m_maxObserved)
                .arg(m_centerOk ? QStringLiteral("OK") : QStringLiteral("--"))
                .arg(m_negTravelOk ? QStringLiteral("OK") : QStringLiteral("--"))
                .arg(m_posTravelOk ? QStringLiteral("OK") : QStringLiteral("--"));

        painter.drawText(card.adjusted(10, 54, -10, -4),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         metricsText);
    }
} // namespace p3d::ui
