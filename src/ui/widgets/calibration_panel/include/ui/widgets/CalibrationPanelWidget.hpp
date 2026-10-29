#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <array>
#include <cstddef>

namespace p3d::ui
{
    // Standalone decoupled widget for interactive device calibration and QSPI Flash persistence.
    // Communicates exclusively via Qt signals and slots (zero coupling to models or transport).
    class CalibrationPanelWidget final : public QWidget
    {
        Q_OBJECT

    public:
        static constexpr std::size_t kAxisCount = 3;

        explicit CalibrationPanelWidget(QWidget* parent = nullptr);
        ~CalibrationPanelWidget() override = default;

        int  deadzoneForAxis(std::size_t axis) const;
        bool invertForAxis(std::size_t axis) const;

        // Accessors for deterministic unit test triggering
        QPushButton* zeroCenterButton() const { return m_zeroBtn; }
        QPushButton* startSweepButton() const { return m_startSweepBtn; }
        QPushButton* endSweepButton() const { return m_endSweepBtn; }
        QPushButton* saveFlashButton() const { return m_saveBtn; }
        QPushButton* loadFlashButton() const { return m_loadBtn; }
        QPushButton* resetFactoryButton() const { return m_resetBtn; }

    public slots:
        void setAxisCalibration(int  axis,
                                int  centerCounts,
                                int  minCounts,
                                int  maxCounts,
                                int  deadzoneCounts,
                                bool invert);
        void setStatusMessage(const QString& statusText);

    signals:
        void zeroCenterRequested();
        void minMaxSweepStartRequested();
        void minMaxSweepEndRequested();
        void saveCalibrationRequested();
        void loadCalibrationRequested();
        void resetCalibrationRequested();
        void deadzoneChanged(int axis, int deadzoneCounts);
        void invertChanged(int axis, bool invert);

    private:
        struct AxisRowControls
        {
            QLabel*    rangeLabel{ nullptr };
            QSpinBox*  deadzoneSpin{ nullptr };
            QCheckBox* invertCheck{ nullptr };
        };

        std::array<AxisRowControls, kAxisCount> m_rows{};
        QPushButton*                            m_zeroBtn{ nullptr };
        QPushButton*                            m_startSweepBtn{ nullptr };
        QPushButton*                            m_endSweepBtn{ nullptr };
        QPushButton*                            m_saveBtn{ nullptr };
        QPushButton*                            m_loadBtn{ nullptr };
        QPushButton*                            m_resetBtn{ nullptr };
        QLabel*                                 m_statusLabel{ nullptr };
    };
} // namespace p3d::ui
