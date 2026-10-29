#include "ui/widgets/GimbalRadarWidget.hpp"

#include <QPainter>
#include <QPen>
#include <QRectF>
#include <algorithm>
#include <cmath>

namespace p3d::ui
{
    GimbalRadarWidget::GimbalRadarWidget(QWidget* parent)
        : QWidget(parent)
    {
        setMinimumSize(240, 240);
    }

    QSize GimbalRadarWidget::minimumSizeHint() const
    {
        return QSize{ 240, 240 };
    }

    QSize GimbalRadarWidget::sizeHint() const
    {
        return QSize{ 300, 300 };
    }

    QPointF GimbalRadarWidget::normalizedDeflection() const
    {
        const double denom = static_cast<double>(std::max(1, m_fullScale));
        const double nx =
            std::clamp(static_cast<double>(m_leftRight) / denom, -1.0, 1.0);
        const double ny =
            std::clamp(static_cast<double>(m_fwdBack) / denom, -1.0, 1.0);
        return QPointF{ nx, ny };
    }

    double GimbalRadarWidget::normalizedRotation() const
    {
        const double denom = static_cast<double>(std::max(1, m_fullScale));
        return std::clamp(static_cast<double>(m_rotation) / denom, -1.0, 1.0);
    }

    void GimbalRadarWidget::setAxesOutput(int fwdBack,
                                          int leftRight,
                                          int rotation,
                                          int fullScale)
    {
        const int clampedScale = std::max(1, fullScale);
        if (m_fwdBack == fwdBack && m_leftRight == leftRight && m_rotation == rotation &&
            m_fullScale == clampedScale)
        {
            return;
        }

        m_fwdBack   = fwdBack;
        m_leftRight = leftRight;
        m_rotation  = rotation;
        m_fullScale = clampedScale;
        update();
    }

    void GimbalRadarWidget::paintEvent(QPaintEvent* /*event*/)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF bounds = rect().adjusted(12.0, 12.0, -12.0, -12.0);
        const double side   = std::min(bounds.width(), bounds.height());
        const QPointF center = bounds.center();
        const double outerRadius = side * 0.46;
        const double radarRadius = side * 0.36;

        // Background card
        painter.setPen(QPen(QColor(48, 54, 61), 1.5));
        painter.setBrush(QColor(18, 22, 28));
        painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 10.0, 10.0);

        // Outer rotation ring track
        const QRectF arcRect(center.x() - outerRadius,
                             center.y() - outerRadius,
                             outerRadius * 2.0,
                             outerRadius * 2.0);
        painter.setPen(QPen(QColor(45, 55, 72), 6.0, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(arcRect, 30 * 16, 120 * 16);

        // Active rotation arc from 90 deg (top center)
        const double rotNorm   = normalizedRotation();
        const int    spanAngle = static_cast<int>(-rotNorm * 60.0 * 16.0);
        painter.setPen(QPen(QColor(56, 189, 248), 6.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawArc(arcRect, 90 * 16, spanAngle);

        // Radar circles and crosshairs
        painter.setPen(QPen(QColor(55, 65, 81), 1.2));
        painter.setBrush(QColor(13, 17, 23));
        painter.drawEllipse(center, radarRadius, radarRadius);
        painter.drawEllipse(center, radarRadius * 0.5, radarRadius * 0.5);

        painter.setPen(QPen(QColor(55, 65, 81), 1.0, Qt::DashLine));
        painter.drawLine(QPointF(center.x() - radarRadius, center.y()),
                         QPointF(center.x() + radarRadius, center.y()));
        painter.drawLine(QPointF(center.x(), center.y() - radarRadius),
                         QPointF(center.x(), center.y() + radarRadius));

        // Puck position (X = Left/Right, Y = -Forward/Backward so Forward moves up)
        const QPointF norm = normalizedDeflection();
        const QPointF puck(center.x() + norm.x() * (radarRadius - 10.0),
                           center.y() - norm.y() * (radarRadius - 10.0));

        // Vector line from center to puck
        painter.setPen(QPen(QColor(16, 185, 129, 180), 2.5));
        painter.drawLine(center, puck);

        // Puck circle
        painter.setPen(QPen(QColor(236, 253, 245), 2.0));
        painter.setBrush(QColor(16, 185, 129));
        painter.drawEllipse(puck, 9.0, 9.0);

        // Caption overlay
        painter.setPen(QColor(201, 209, 217));
        QFont f = font();
        f.setPointSize(9);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(rect().adjusted(10, 6, -10, -6),
                         Qt::AlignTop | Qt::AlignLeft,
                         QStringLiteral("3-DOF GIMBAL RADAR"));

        f.setBold(false);
        painter.setFont(f);
        painter.drawText(
            rect().adjusted(10, 6, -10, -8),
            Qt::AlignBottom | Qt::AlignHCenter,
            QStringLiteral("FWD:%1  L/R:%2  ROT:%3").arg(m_fwdBack).arg(m_leftRight).arg(m_rotation));
    }
} // namespace p3d::ui
