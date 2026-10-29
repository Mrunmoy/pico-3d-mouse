#include "ui/widgets/CalibrationPanelWidget.hpp"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace p3d::ui
{
    CalibrationPanelWidget::CalibrationPanelWidget(QWidget* parent)
        : QWidget(parent)
    {
        auto* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);

        auto* group = new QGroupBox(QStringLiteral("Calibration & QSPI Flash Persistence"), this);
        auto* vbox  = new QVBoxLayout(group);

        // Workflow buttons
        auto* btnRow1   = new QHBoxLayout();
        m_zeroBtn       = new QPushButton(QStringLiteral("1. Zero Center"), group);
        m_startSweepBtn = new QPushButton(QStringLiteral("2. Start Min/Max Sweep"), group);
        m_endSweepBtn   = new QPushButton(QStringLiteral("3. Finish Sweep"), group);
        btnRow1->addWidget(m_zeroBtn);
        btnRow1->addWidget(m_startSweepBtn);
        btnRow1->addWidget(m_endSweepBtn);
        vbox->addLayout(btnRow1);

        auto* btnRow2 = new QHBoxLayout();
        m_saveBtn     = new QPushButton(QStringLiteral("Save to QSPI Flash"), group);
        m_loadBtn     = new QPushButton(QStringLiteral("Load from Flash"), group);
        m_resetBtn    = new QPushButton(QStringLiteral("Factory Defaults"), group);
        btnRow2->addWidget(m_saveBtn);
        btnRow2->addWidget(m_loadBtn);
        btnRow2->addWidget(m_resetBtn);
        vbox->addLayout(btnRow2);

        // Per-axis tuning table
        auto* grid = new QGridLayout();
        grid->addWidget(new QLabel(QStringLiteral("<b>Axis</b>"), group), 0, 0);
        grid->addWidget(new QLabel(QStringLiteral("<b>Calibrated [Min / Center / Max]</b>"), group), 0, 1);
        grid->addWidget(new QLabel(QStringLiteral("<b>Deadzone</b>"), group), 0, 2);
        grid->addWidget(new QLabel(QStringLiteral("<b>Invert</b>"), group), 0, 3);

        static const std::array<QString, kAxisCount> kAxisNames = {
            QStringLiteral("CH0: Forward / Backward (ADC0)"),
            QStringLiteral("CH1: Left / Right (ADC1)"),
            QStringLiteral("CH2: Rotation / Twist (ADC2)")
        };

        for (std::size_t i = 0; i < kAxisCount; ++i)
        {
            const int rowIdx = static_cast<int>(i) + 1;
            grid->addWidget(new QLabel(kAxisNames[i], group), rowIdx, 0);

            m_rows[i].rangeLabel =
                new QLabel(QStringLiteral("[0 / 2048 / 4095]"), group);
            grid->addWidget(m_rows[i].rangeLabel, rowIdx, 1);

            m_rows[i].deadzoneSpin = new QSpinBox(group);
            m_rows[i].deadzoneSpin->setRange(0, 1000);
            m_rows[i].deadzoneSpin->setValue(12);
            m_rows[i].deadzoneSpin->setSuffix(QStringLiteral(" cts"));
            grid->addWidget(m_rows[i].deadzoneSpin, rowIdx, 2);

            m_rows[i].invertCheck = new QCheckBox(QStringLiteral("Invert"), group);
            grid->addWidget(m_rows[i].invertCheck, rowIdx, 3);

            const int axisIndex = static_cast<int>(i);
            connect(m_rows[i].deadzoneSpin,
                    &QSpinBox::valueChanged,
                    this,
                    [this, axisIndex](int val) { emit deadzoneChanged(axisIndex, val); });
            connect(m_rows[i].invertCheck,
                    &QCheckBox::toggled,
                    this,
                    [this, axisIndex](bool checked) { emit invertChanged(axisIndex, checked); });
        }

        vbox->addLayout(grid);

        m_statusLabel = new QLabel(QStringLiteral("Calibration status: Ready"), group);
        vbox->addWidget(m_statusLabel);

        rootLayout->addWidget(group);

        connect(m_zeroBtn,
                &QPushButton::clicked,
                this,
                &CalibrationPanelWidget::zeroCenterRequested);
        connect(m_startSweepBtn,
                &QPushButton::clicked,
                this,
                &CalibrationPanelWidget::minMaxSweepStartRequested);
        connect(m_endSweepBtn,
                &QPushButton::clicked,
                this,
                &CalibrationPanelWidget::minMaxSweepEndRequested);
        connect(m_saveBtn,
                &QPushButton::clicked,
                this,
                &CalibrationPanelWidget::saveCalibrationRequested);
        connect(m_loadBtn,
                &QPushButton::clicked,
                this,
                &CalibrationPanelWidget::loadCalibrationRequested);
        connect(m_resetBtn,
                &QPushButton::clicked,
                this,
                &CalibrationPanelWidget::resetCalibrationRequested);
    }

    int CalibrationPanelWidget::deadzoneForAxis(std::size_t axis) const
    {
        return m_rows.at(axis < kAxisCount ? axis : 0).deadzoneSpin->value();
    }

    bool CalibrationPanelWidget::invertForAxis(std::size_t axis) const
    {
        return m_rows.at(axis < kAxisCount ? axis : 0).invertCheck->isChecked();
    }

    void CalibrationPanelWidget::setAxisCalibration(int  axis,
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
        auto& row = m_rows[static_cast<std::size_t>(axis)];
        row.rangeLabel->setText(
            QStringLiteral("[%1 / %2 / %3]").arg(minCounts).arg(centerCounts).arg(maxCounts));

        const QSignalBlocker blockDz(row.deadzoneSpin);
        const QSignalBlocker blockInv(row.invertCheck);
        row.deadzoneSpin->setValue(deadzoneCounts);
        row.invertCheck->setChecked(invert);
    }

    void CalibrationPanelWidget::setStatusMessage(const QString& statusText)
    {
        m_statusLabel->setText(statusText);
    }
} // namespace p3d::ui
