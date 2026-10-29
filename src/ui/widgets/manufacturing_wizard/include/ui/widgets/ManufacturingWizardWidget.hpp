#pragma once

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QWidget>
#include <array>
#include <cstddef>
#include <cstdint>

namespace p3d::ui
{
    // Guided 6-step Manufacturing & Calibration Wizard widget.
    // Walks the operator through:
    //   Step 0: Connect USB-C Device (or Flash Firmware)
    //   Step 1: Hands-Off Resting Center Calibration (ZERO)
    //   Step 2: 6-Direction Full Mechanical Travel Sweep (CAL_MINMAX_START -> CAL_MINMAX_END)
    //   Step 3: Deadzone & Axis Polarity Verification on 3-DOF Radar
    //   Step 4: Save Calibration to RP2040 QSPI Flash (SAVE_CAL -> @CAL SAVED_OK)
    //   Step 5: Manufacturing PASS Certificate & "Test Next Unit"
    //
    // Strictly decoupled: communicates exclusively via Qt Signals & Slots.
    class ManufacturingWizardWidget final : public QWidget
    {
        Q_OBJECT

    public:
        enum class Step : int
        {
            ConnectDevice     = 0,
            CenterCalibration = 1,
            RangeSweep        = 2,
            VerifyAndTune     = 3,
            SaveToFlash       = 4,
            CompletedPass     = 5
        };

        static constexpr std::size_t kStepCount = 6;
        static constexpr std::size_t kAxisCount = 3;

        explicit ManufacturingWizardWidget(QWidget* parent = nullptr);
        ~ManufacturingWizardWidget() override = default;

        Step currentStep() const { return m_currentStep; }
        int  deadzoneForAxis(std::size_t axis) const;
        bool invertForAxis(std::size_t axis) const;

        // Button accessors for deterministic off-target unit tests
        QPushButton* captureCenterButton() const { return m_captureCenterBtn; }
        QPushButton* finishSweepButton() const { return m_finishSweepBtn; }
        QPushButton* confirmTuningButton() const { return m_confirmTuningBtn; }
        QPushButton* saveToFlashButton() const { return m_saveFlashBtn; }
        QPushButton* testNextUnitButton() const { return m_testNextUnitBtn; }

    public slots:
        void setConnected(bool connected, const QString& portName);
        void setStep(Step step);
        void updateAxisSweepProgress(std::size_t axis,
                                     bool        centerInSpec,
                                     bool        negativeReached,
                                     bool        positiveReached,
                                     std::uint16_t minObserved,
                                     std::uint16_t maxObserved);
        void setAxisCalibration(int  axis,
                                int  centerCounts,
                                int  minCounts,
                                int  maxCounts,
                                int  deadzoneCounts,
                                bool invert);
        void onDeviceCalibrationEvent(const QString& eventName);

    signals:
        void connectDeviceRequested();
        void openFirmwareUpdaterRequested();
        void zeroCenterRequested();
        void minMaxSweepStartRequested();
        void minMaxSweepEndRequested();
        void deadzoneChanged(int axis, int deadzoneCounts);
        void invertChanged(int axis, bool invert);
        void saveCalibrationRequested();
        void restartWizardRequested();

    private:
        void refreshStepHeader();
        void updateSweepChecklistUi();

        Step                                    m_currentStep{ Step::ConnectDevice };
        bool                                    m_isConnected{ false };
        std::array<QLabel*, kStepCount>         m_stepPills{};
        QLabel*                                 m_stepTitleLabel{ nullptr };
        QLabel*                                 m_stepInstructionLabel{ nullptr };
        QStackedWidget*                         m_pages{ nullptr };

        // Page 0: Connect
        QPushButton* m_connectDeviceBtn{ nullptr };
        QPushButton* m_openFwUpdaterBtn{ nullptr };

        // Page 1: Center Calibration
        QLabel*      m_centerCheckLabel{ nullptr };
        QPushButton* m_captureCenterBtn{ nullptr };

        // Page 2: 6-Direction Sweep
        struct SweepDirectionRow
        {
            QLabel* negBadge{ nullptr };
            QLabel* posBadge{ nullptr };
            QLabel* spanLabel{ nullptr };
            bool    negDone{ false };
            bool    posDone{ false };
        };
        std::array<SweepDirectionRow, kAxisCount> m_sweepRows{};
        QPushButton*                              m_finishSweepBtn{ nullptr };

        // Page 3: Verify & Tune
        struct TuneRow
        {
            QSpinBox*  deadzoneSpin{ nullptr };
            QCheckBox* invertCheck{ nullptr };
        };
        std::array<TuneRow, kAxisCount> m_tuneRows{};
        QPushButton*                    m_confirmTuningBtn{ nullptr };

        // Page 4: Save to QSPI Flash
        QLabel*      m_saveSummaryLabel{ nullptr };
        QPushButton* m_saveFlashBtn{ nullptr };

        // Page 5: Completed Pass
        QLabel*                               m_passDetailsLabel{ nullptr };
        QPushButton*                          m_testNextUnitBtn{ nullptr };
        std::array<QString, kAxisCount>       m_calSummaryLines{};
    };
} // namespace p3d::ui
