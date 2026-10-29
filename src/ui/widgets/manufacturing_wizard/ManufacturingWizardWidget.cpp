#include "ui/widgets/ManufacturingWizardWidget.hpp"

#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace p3d::ui
{
    ManufacturingWizardWidget::ManufacturingWizardWidget(QWidget* parent)
        : QWidget(parent)
    {
        auto* root = new QVBoxLayout(this);
        root->setSpacing(10);

        // ── Top 6-Step Progress Bar ──────────────────────────────────
        auto* pillsFrame  = new QFrame(this);
        auto* pillsLayout = new QHBoxLayout(pillsFrame);
        pillsLayout->setContentsMargins(0, 0, 0, 0);
        pillsLayout->setSpacing(6);

        static const std::array<QString, kStepCount> kStepNames = {
            QStringLiteral("1. Connect"),
            QStringLiteral("2. Zero Center"),
            QStringLiteral("3. Sweep Travel"),
            QStringLiteral("4. Tune & Verify"),
            QStringLiteral("5. Save Flash"),
            QStringLiteral("6. Done / Pass")
        };

        for (std::size_t i = 0; i < kStepCount; ++i)
        {
            m_stepPills[i] = new QLabel(kStepNames[i], pillsFrame);
            m_stepPills[i]->setAlignment(Qt::AlignCenter);
            m_stepPills[i]->setMinimumHeight(30);
            pillsLayout->addWidget(m_stepPills[i], 1);
        }
        root->addWidget(pillsFrame);

        // ── Active Step Instruction Card ─────────────────────────────
        auto* cardBox    = new QGroupBox(this);
        auto* cardLayout = new QVBoxLayout(cardBox);
        cardLayout->setSpacing(10);

        m_stepTitleLabel = new QLabel(cardBox);
        QFont titleFont  = font();
        titleFont.setPointSize(12);
        titleFont.setBold(true);
        m_stepTitleLabel->setFont(titleFont);
        cardLayout->addWidget(m_stepTitleLabel);

        m_stepInstructionLabel = new QLabel(cardBox);
        m_stepInstructionLabel->setWordWrap(true);
        m_stepInstructionLabel->setStyleSheet(
            QStringLiteral("QLabel { color: #D1D5DB; font-size: 13px; padding: 4px 0; }"));
        cardLayout->addWidget(m_stepInstructionLabel);

        m_pages = new QStackedWidget(cardBox);

        // ── Page 0: Connect Device ───────────────────────────────────
        {
            auto* page   = new QWidget(m_pages);
            auto* layout = new QVBoxLayout(page);
            m_connectDeviceBtn = new QPushButton(
                QStringLiteral("Connect to USB-C Serial Port →"), page);
            m_connectDeviceBtn->setMinimumHeight(40);
            m_connectDeviceBtn->setStyleSheet(
                QStringLiteral("QPushButton { background-color: #2563EB; color: white; font-weight: bold; "
                               "padding: 8px 16px; border-radius: 6px; }"));

            m_openFwUpdaterBtn = new QPushButton(
                QStringLiteral("Need to Flash / Update Firmware First? Open Buttonless USB Updater"),
                page);
            m_openFwUpdaterBtn->setMinimumHeight(34);

            layout->addWidget(m_connectDeviceBtn);
            layout->addWidget(m_openFwUpdaterBtn);
            layout->addStretch(1);
            m_pages->addWidget(page);

            connect(m_connectDeviceBtn,
                    &QPushButton::clicked,
                    this,
                    &ManufacturingWizardWidget::connectDeviceRequested);
            connect(m_openFwUpdaterBtn,
                    &QPushButton::clicked,
                    this,
                    &ManufacturingWizardWidget::openFirmwareUpdaterRequested);
        }

        // ── Page 1: Center Calibration (Hands Off) ───────────────────
        {
            auto* page   = new QWidget(m_pages);
            auto* layout = new QVBoxLayout(page);

            m_centerCheckLabel = new QLabel(
                QStringLiteral("Waiting for resting ADC readings near mid-scale (~1.65 V / 2048 counts)..."),
                page);
            m_centerCheckLabel->setWordWrap(true);
            layout->addWidget(m_centerCheckLabel);

            m_captureCenterBtn = new QPushButton(
                QStringLiteral("Hands Off Joystick — Capture Resting Center Now →"), page);
            m_captureCenterBtn->setMinimumHeight(42);
            m_captureCenterBtn->setStyleSheet(
                QStringLiteral("QPushButton { background-color: #059669; color: white; font-weight: bold; "
                               "padding: 8px 16px; border-radius: 6px; }"));
            layout->addWidget(m_captureCenterBtn);
            layout->addStretch(1);
            m_pages->addWidget(page);

            connect(m_captureCenterBtn,
                    &QPushButton::clicked,
                    this,
                    &ManufacturingWizardWidget::zeroCenterRequested);
        }

        // ── Page 2: 6-Direction Mechanical Range Sweep ───────────────
        {
            auto* page   = new QWidget(m_pages);
            auto* layout = new QVBoxLayout(page);

            auto* grid = new QGridLayout();
            grid->addWidget(new QLabel(QStringLiteral("<b>Axis</b>"), page), 0, 0);
            grid->addWidget(new QLabel(QStringLiteral("<b>Negative End</b>"), page), 0, 1);
            grid->addWidget(new QLabel(QStringLiteral("<b>Positive End</b>"), page), 0, 2);
            grid->addWidget(new QLabel(QStringLiteral("<b>Observed Range</b>"), page), 0, 3);

            static const std::array<QString, kAxisCount> kAxisTitles = {
                QStringLiteral("Forward / Backward (ADC0)"),
                QStringLiteral("Left / Right (ADC1)"),
                QStringLiteral("Rotation / Twist (ADC2)")
            };
            static const std::array<QString, kAxisCount> kNegNames = {
                QStringLiteral("Pull Backward"),
                QStringLiteral("Push Left"),
                QStringLiteral("Twist CCW")
            };
            static const std::array<QString, kAxisCount> kPosNames = {
                QStringLiteral("Push Forward"),
                QStringLiteral("Push Right"),
                QStringLiteral("Twist CW")
            };

            for (std::size_t i = 0; i < kAxisCount; ++i)
            {
                const int r = static_cast<int>(i) + 1;
                grid->addWidget(new QLabel(kAxisTitles[i], page), r, 0);

                m_sweepRows[i].negBadge = new QLabel(
                    QStringLiteral("⏳ %1").arg(kNegNames[i]), page);
                m_sweepRows[i].posBadge = new QLabel(
                    QStringLiteral("⏳ %1").arg(kPosNames[i]), page);
                m_sweepRows[i].spanLabel = new QLabel(QStringLiteral("[2048 .. 2048]"), page);

                grid->addWidget(m_sweepRows[i].negBadge, r, 1);
                grid->addWidget(m_sweepRows[i].posBadge, r, 2);
                grid->addWidget(m_sweepRows[i].spanLabel, r, 3);
            }
            layout->addLayout(grid);

            m_finishSweepBtn = new QPushButton(
                QStringLiteral("Lock In Min/Max Travel Range & Continue →"), page);
            m_finishSweepBtn->setMinimumHeight(42);
            m_finishSweepBtn->setStyleSheet(
                QStringLiteral("QPushButton { background-color: #2563EB; color: white; font-weight: bold; "
                               "padding: 8px 16px; border-radius: 6px; }"));
            layout->addWidget(m_finishSweepBtn);
            layout->addStretch(1);
            m_pages->addWidget(page);

            connect(m_finishSweepBtn,
                    &QPushButton::clicked,
                    this,
                    &ManufacturingWizardWidget::minMaxSweepEndRequested);
        }

        // ── Page 3: Deadzone & Polarity Verification ─────────────────
        {
            auto* page   = new QWidget(m_pages);
            auto* layout = new QVBoxLayout(page);

            auto* grid = new QGridLayout();
            grid->addWidget(new QLabel(QStringLiteral("<b>Axis</b>"), page), 0, 0);
            grid->addWidget(new QLabel(QStringLiteral("<b>Center Deadzone</b>"), page), 0, 1);
            grid->addWidget(new QLabel(QStringLiteral("<b>Direction Polarity</b>"), page), 0, 2);

            static const std::array<QString, kAxisCount> kAxisNames = {
                QStringLiteral("Forward / Backward (ADC0)"),
                QStringLiteral("Left / Right (ADC1)"),
                QStringLiteral("Rotation / Twist (ADC2)")
            };

            for (std::size_t i = 0; i < kAxisCount; ++i)
            {
                const int r = static_cast<int>(i) + 1;
                grid->addWidget(new QLabel(kAxisNames[i], page), r, 0);

                m_tuneRows[i].deadzoneSpin = new QSpinBox(page);
                m_tuneRows[i].deadzoneSpin->setRange(0, 1000);
                m_tuneRows[i].deadzoneSpin->setValue(12);
                m_tuneRows[i].deadzoneSpin->setSuffix(QStringLiteral(" counts"));
                grid->addWidget(m_tuneRows[i].deadzoneSpin, r, 1);

                m_tuneRows[i].invertCheck =
                    new QCheckBox(QStringLiteral("Invert Direction"), page);
                grid->addWidget(m_tuneRows[i].invertCheck, r, 2);

                const int axisIdx = static_cast<int>(i);
                connect(m_tuneRows[i].deadzoneSpin,
                        &QSpinBox::valueChanged,
                        this,
                        [this, axisIdx](int val) { emit deadzoneChanged(axisIdx, val); });
                connect(m_tuneRows[i].invertCheck,
                        &QCheckBox::toggled,
                        this,
                        [this, axisIdx](bool inv) { emit invertChanged(axisIdx, inv); });
            }
            layout->addLayout(grid);

            m_confirmTuningBtn = new QPushButton(
                QStringLiteral("Directions & Deadzone Verified — Proceed to Save →"), page);
            m_confirmTuningBtn->setMinimumHeight(42);
            m_confirmTuningBtn->setStyleSheet(
                QStringLiteral("QPushButton { background-color: #059669; color: white; font-weight: bold; "
                               "padding: 8px 16px; border-radius: 6px; }"));
            layout->addWidget(m_confirmTuningBtn);
            layout->addStretch(1);
            m_pages->addWidget(page);

            connect(m_confirmTuningBtn, &QPushButton::clicked, this, [this]() {
                setStep(Step::SaveToFlash);
            });
        }

        // ── Page 4: Save Calibration to QSPI Flash ───────────────────
        {
            auto* page   = new QWidget(m_pages);
            auto* layout = new QVBoxLayout(page);

            m_saveSummaryLabel = new QLabel(
                QStringLiteral("Ready to write calibration + CRC-32 checksum to RP2040 QSPI Flash."),
                page);
            m_saveSummaryLabel->setWordWrap(true);
            layout->addWidget(m_saveSummaryLabel);

            m_saveFlashBtn = new QPushButton(
                QStringLiteral("Save Calibration Permanently to RP2040 Flash →"), page);
            m_saveFlashBtn->setMinimumHeight(44);
            m_saveFlashBtn->setStyleSheet(
                QStringLiteral("QPushButton { background-color: #059669; color: white; font-weight: bold; "
                               "padding: 8px 16px; border-radius: 6px; }"));
            layout->addWidget(m_saveFlashBtn);
            layout->addStretch(1);
            m_pages->addWidget(page);

            connect(m_saveFlashBtn,
                    &QPushButton::clicked,
                    this,
                    &ManufacturingWizardWidget::saveCalibrationRequested);
        }

        // ── Page 5: Manufacturing PASS Certificate ───────────────────
        {
            auto* page   = new QWidget(m_pages);
            auto* layout = new QVBoxLayout(page);

            auto* passBanner = new QLabel(
                QStringLiteral("✅  UNIT PASSED MANUFACTURING TEST & FLASH CALIBRATION"), page);
            passBanner->setAlignment(Qt::AlignCenter);
            passBanner->setMinimumHeight(46);
            passBanner->setStyleSheet(
                QStringLiteral("QLabel { background-color: #065F46; color: #D1FAE5; font-size: 14px; "
                               "font-weight: bold; border-radius: 8px; padding: 8px; }"));
            layout->addWidget(passBanner);

            m_passDetailsLabel = new QLabel(page);
            m_passDetailsLabel->setWordWrap(true);
            layout->addWidget(m_passDetailsLabel);

            m_testNextUnitBtn = new QPushButton(
                QStringLiteral("Start Calibration & Test for Next Unit ↻"), page);
            m_testNextUnitBtn->setMinimumHeight(40);
            layout->addWidget(m_testNextUnitBtn);
            layout->addStretch(1);
            m_pages->addWidget(page);

            connect(m_testNextUnitBtn, &QPushButton::clicked, this, [this]() {
                emit restartWizardRequested();
                setStep(m_isConnected ? Step::CenterCalibration : Step::ConnectDevice);
            });
        }

        cardLayout->addWidget(m_pages);

        // Navigation footer (Back / Skip for operator flexibility)
        auto* navRow  = new QHBoxLayout();
        auto* backBtn = new QPushButton(QStringLiteral("← Previous Step"), cardBox);
        auto* nextBtn = new QPushButton(QStringLiteral("Skip Step →"), cardBox);
        navRow->addWidget(backBtn);
        navRow->addStretch(1);
        navRow->addWidget(nextBtn);
        cardLayout->addLayout(navRow);

        connect(backBtn, &QPushButton::clicked, this, [this]() {
            const int idx = static_cast<int>(m_currentStep);
            if (idx > 0)
            {
                setStep(static_cast<Step>(idx - 1));
            }
        });
        connect(nextBtn, &QPushButton::clicked, this, [this]() {
            const int idx = static_cast<int>(m_currentStep);
            if (idx + 1 < static_cast<int>(kStepCount))
            {
                setStep(static_cast<Step>(idx + 1));
            }
        });

        root->addWidget(cardBox, 1);

        refreshStepHeader();
    }

    int ManufacturingWizardWidget::deadzoneForAxis(std::size_t axis) const
    {
        return m_tuneRows.at(axis < kAxisCount ? axis : 0).deadzoneSpin->value();
    }

    bool ManufacturingWizardWidget::invertForAxis(std::size_t axis) const
    {
        return m_tuneRows.at(axis < kAxisCount ? axis : 0).invertCheck->isChecked();
    }

    void ManufacturingWizardWidget::setConnected(bool connected, const QString& /*portName*/)
    {
        m_isConnected = connected;
        if (connected && m_currentStep == Step::ConnectDevice)
        {
            setStep(Step::CenterCalibration);
        }
        else if (!connected)
        {
            setStep(Step::ConnectDevice);
        }
    }

    void ManufacturingWizardWidget::setStep(Step step)
    {
        if (m_currentStep == step)
        {
            refreshStepHeader();
            return;
        }

        m_currentStep = step;
        m_pages->setCurrentIndex(static_cast<int>(step));
        refreshStepHeader();

        if (step == Step::RangeSweep)
        {
            emit minMaxSweepStartRequested();
        }
    }

    void ManufacturingWizardWidget::updateAxisSweepProgress(std::size_t   axis,
                                                            bool          centerInSpec,
                                                            bool          negativeReached,
                                                            bool          positiveReached,
                                                            std::uint16_t minObserved,
                                                            std::uint16_t maxObserved)
    {
        if (axis >= kAxisCount)
        {
            return;
        }

        if (axis == 0)
        {
            m_centerCheckLabel->setText(
                centerInSpec
                    ? QStringLiteral("✅ Resting voltages are within valid mid-scale tolerance (~1.65 V).")
                    : QStringLiteral("⚠️ Check joystick resting position — waiting for center within [1400..2700]."));
        }

        auto& row    = m_sweepRows[axis];
        row.negDone  = negativeReached;
        row.posDone  = positiveReached;
        row.spanLabel->setText(
            QStringLiteral("[%1 .. %2]").arg(minObserved).arg(maxObserved));

        static const std::array<QString, kAxisCount> kNegNames = {
            QStringLiteral("Pull Backward"),
            QStringLiteral("Push Left"),
            QStringLiteral("Twist CCW")
        };
        static const std::array<QString, kAxisCount> kPosNames = {
            QStringLiteral("Push Forward"),
            QStringLiteral("Push Right"),
            QStringLiteral("Twist CW")
        };

        row.negBadge->setText(
            QStringLiteral("%1 %2")
                .arg(negativeReached ? QStringLiteral("✅") : QStringLiteral("⏳"),
                     kNegNames[axis]));
        row.posBadge->setText(
            QStringLiteral("%1 %2")
                .arg(positiveReached ? QStringLiteral("✅") : QStringLiteral("⏳"),
                     kPosNames[axis]));
    }

    void ManufacturingWizardWidget::setAxisCalibration(int  axis,
                                                       int  centerCounts,
                                                       int  minCounts,
                                                       int  maxCounts,
                                                       int  deadzoneCounts,
                                                       bool invert)
    {
        if (axis < 0 || static_cast<std::size_t>(axis) >= kAxisCount)
        {
            return;
        }

        const auto idx = static_cast<std::size_t>(axis);
        {
            const QSignalBlocker b1(m_tuneRows[idx].deadzoneSpin);
            const QSignalBlocker b2(m_tuneRows[idx].invertCheck);
            m_tuneRows[idx].deadzoneSpin->setValue(deadzoneCounts);
            m_tuneRows[idx].invertCheck->setChecked(invert);
        }

        static const std::array<QString, kAxisCount> kNames = {
            QStringLiteral("CH0 Forward/Backward (ADC0)"),
            QStringLiteral("CH1 Left/Right (ADC1)"),
            QStringLiteral("CH2 Rotation/Twist (ADC2)")
        };

        m_calSummaryLines[idx] =
            QStringLiteral("• %1: Center=%2, Min=%3, Max=%4, Deadzone=%5, Invert=%6")
                .arg(kNames[idx])
                .arg(centerCounts)
                .arg(minCounts)
                .arg(maxCounts)
                .arg(deadzoneCounts)
                .arg(invert ? QStringLiteral("Yes") : QStringLiteral("No"));

        const QString combined = QStringLiteral("%1\n%2\n%3")
                                     .arg(m_calSummaryLines[0],
                                          m_calSummaryLines[1],
                                          m_calSummaryLines[2]);
        m_saveSummaryLabel->setText(combined);
        m_passDetailsLabel->setText(
            QStringLiteral("<b>Saved QSPI Flash Calibration (CRC-32 Verified):</b><br/><pre>%1</pre>")
                .arg(combined));
    }

    void ManufacturingWizardWidget::onDeviceCalibrationEvent(const QString& eventName)
    {
        if (eventName == QStringLiteral("ZERO_OK") &&
            m_currentStep == Step::CenterCalibration)
        {
            setStep(Step::RangeSweep);
        }
        else if (eventName == QStringLiteral("MINMAX_END") &&
                 m_currentStep == Step::RangeSweep)
        {
            setStep(Step::VerifyAndTune);
        }
        else if (eventName == QStringLiteral("SAVED_OK") &&
                 m_currentStep == Step::SaveToFlash)
        {
            setStep(Step::CompletedPass);
        }
    }

    void ManufacturingWizardWidget::refreshStepHeader()
    {
        const int activeIdx = static_cast<int>(m_currentStep);

        for (std::size_t i = 0; i < kStepCount; ++i)
        {
            const int idx = static_cast<int>(i);
            if (idx < activeIdx)
            {
                m_stepPills[i]->setStyleSheet(
                    QStringLiteral("QLabel { background-color: #065F46; color: #D1FAE5; "
                                   "font-weight: bold; border-radius: 5px; padding: 4px; }"));
            }
            else if (idx == activeIdx)
            {
                m_stepPills[i]->setStyleSheet(
                    QStringLiteral("QLabel { background-color: #2563EB; color: #FFFFFF; "
                                   "font-weight: bold; border-radius: 5px; padding: 4px; }"));
            }
            else
            {
                m_stepPills[i]->setStyleSheet(
                    QStringLiteral("QLabel { background-color: #1F2937; color: #9CA3AF; "
                                   "border-radius: 5px; padding: 4px; }"));
            }
        }

        switch (m_currentStep)
        {
            case Step::ConnectDevice:
                m_stepTitleLabel->setText(
                    QStringLiteral("Step 1 of 6 — Connect Your pico-3d-mouse Over USB-C"));
                m_stepInstructionLabel->setText(
                    QStringLiteral("Plug the Raspberry Pi Pico into this computer over USB-C. "
                                   "Select its serial port (/dev/ttyACM0) in the top bar and click Connect below."));
                break;

            case Step::CenterCalibration:
                m_stepTitleLabel->setText(
                    QStringLiteral("Step 2 of 6 — Hands-Off Resting Center Calibration"));
                m_stepInstructionLabel->setText(
                    QStringLiteral("Let go of the 3D mouse knob completely so all 3 potentiometers rest at their neutral "
                                   "center position. Then click the green button below to record the center zero point."));
                break;

            case Step::RangeSweep:
                m_stepTitleLabel->setText(
                    QStringLiteral("Step 3 of 6 — Sweep All 3 Axes to Their Full Mechanical Limits"));
                m_stepInstructionLabel->setText(
                    QStringLiteral("Move the joystick all the way Forward, Backward, Left, Right, and Twist it fully "
                                   "both ways until all 6 checkmarks turn green (✅). Then click Lock In Range."));
                break;

            case Step::VerifyAndTune:
                m_stepTitleLabel->setText(
                    QStringLiteral("Step 4 of 6 — Verify Directions on the 3-DOF Radar & Tune Deadzone"));
                m_stepInstructionLabel->setText(
                    QStringLiteral("Move the knob and watch the 3-DOF Gimbal Radar on the left. If any axis moves in the "
                                   "opposite direction of your hand, check 'Invert Direction' for that axis."));
                break;

            case Step::SaveToFlash:
                m_stepTitleLabel->setText(
                    QStringLiteral("Step 5 of 6 — Save Calibration to RP2040 Internal QSPI Flash"));
                m_stepInstructionLabel->setText(
                    QStringLiteral("Click the button below to permanently store this calibration into the Pico's "
                                   "non-volatile flash memory so it automatically loads on every power-up."));
                break;

            case Step::CompletedPass:
                m_stepTitleLabel->setText(
                    QStringLiteral("Step 6 of 6 — Manufacturing Test & Calibration Complete!"));
                m_stepInstructionLabel->setText(
                    QStringLiteral("Your pico-3d-mouse is calibrated, verified, and actively streaming 6-DOF USB HID reports. "
                                   "You can unplug the device or click below to calibrate another unit."));
                break;
        }
    }
} // namespace p3d::ui
