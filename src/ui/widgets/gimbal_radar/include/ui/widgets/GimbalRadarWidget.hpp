#pragma once

#include <QPointF>
#include <QWidget>

namespace p3d::ui
{
    // Standalone custom QPainter widget rendering 3-DOF joystick deflection:
    //   - X/Y puck position inside a radar reticle (Left/Right vs Forward/Backward)
    //   - Outer concentric arc showing Rotation (Twist) deflection
    // Zero coupling to models, controllers, or serial transport.
    class GimbalRadarWidget final : public QWidget
    {
        Q_OBJECT

    public:
        explicit GimbalRadarWidget(QWidget* parent = nullptr);
        ~GimbalRadarWidget() override = default;

        int     forwardBackward() const { return m_fwdBack; }
        int     leftRight() const { return m_leftRight; }
        int     rotation() const { return m_rotation; }
        int     fullScale() const { return m_fullScale; }
        QPointF normalizedDeflection() const;
        double  normalizedRotation() const;

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

    public slots:
        void setAxesOutput(int fwdBack, int leftRight, int rotation, int fullScale = 500);

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        int m_fwdBack{ 0 };
        int m_leftRight{ 0 };
        int m_rotation{ 0 };
        int m_fullScale{ 500 };
    };
} // namespace p3d::ui
