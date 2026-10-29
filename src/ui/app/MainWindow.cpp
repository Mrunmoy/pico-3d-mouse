#include "ui/app/MainWindow.hpp"

#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QVBoxLayout>

namespace p3d::ui
{
    MainWindow::MainWindow(ISerialTransport&        serialTransport,
                           IFirmwareUpdater&        firmwareUpdater,
                           DeviceTelemetryModel&    telemetryModel,
                           ManufacturingController& controller,
                           QWidget*                 parent)
        : QMainWindow(parent)
        , m_serial(serialTransport)
        , m_updater(firmwareUpdater)
        , m_model(telemetryModel)
        , m_controller(controller)
    {
        setWindowTitle(
            QStringLiteral("pico-3d-mouse — Manufacturing Tester, Calibration & Buttonless Firmware Updater"));
        resize(1080, 720);

        auto* central = new QWidget(this);
        auto* root    = new QVBoxLayout(central);

        // ── Top Connection & Manufacturing Verdict Bar ───────────────
        auto* topBar = new QHBoxLayout();
        topBar->addWidget(new QLabel(QStringLiteral("<b>USB-C Serial Port:</b>"), central));

        m_portCombo = new QComboBox(central);
        m_portCombo->setEditable(true);
        m_portCombo->setMinimumWidth(200);
        topBar->addWidget(m_portCombo);

        m_refreshBtn = new QPushButton(QStringLiteral("Refresh Ports"), central);
        m_connectBtn = new QPushButton(QStringLiteral("Connect"), central);
        m_resetTestBtn = new QPushButton(QStringLiteral("Reset Mfg Test"), central);
        topBar->addWidget(m_refreshBtn);
        topBar->addWidget(m_connectBtn);
        topBar->addWidget(m_resetTestBtn);
        topBar->addStretch(1);

        m_verdictBadge = new QLabel(
            QStringLiteral("MFG TEST: IN PROGRESS (SWEEP ALL 3 AXES)"), central);
        m_verdictBadge->setAlignment(Qt::AlignCenter);
        m_verdictBadge->setMinimumWidth(310);
        m_verdictBadge->setStyleSheet(
            QStringLiteral("QLabel { background-color: #374151; color: #F9FAFB; font-weight: bold; "
                           "padding: 6px 12px; border-radius: 6px; }"));
        topBar->addWidget(m_verdictBadge);
        root->addLayout(topBar);

        // ── Main Content Split: Telemetry/Radar (Left) vs Calibration/Updater (Right) ──
        auto* contentRow = new QHBoxLayout();

        auto* leftCol = new QVBoxLayout();
        m_gauges[0]   = new AxisGaugeWidget(
            QStringLiteral("Forward / Backward"), QStringLiteral("ADC0 / GP26 / Pin 31"), central);
        m_gauges[1] = new AxisGaugeWidget(
            QStringLiteral("Left / Right"), QStringLiteral("ADC1 / GP27 / Pin 32"), central);
        m_gauges[2] = new AxisGaugeWidget(
            QStringLiteral("Rotation / Twist"), QStringLiteral("ADC2 / GP28 / Pin 34"), central);

        for (auto* gauge : m_gauges)
        {
            leftCol->addWidget(gauge);
        }

        m_radar = new GimbalRadarWidget(central);
        leftCol->addWidget(m_radar, 1);
        contentRow->addLayout(leftCol, 1);

        auto* rightCol = new QVBoxLayout();
        m_calPanel     = new CalibrationPanelWidget(central);
        m_fwWidget     = new FirmwareUpdaterWidget(central);

        const QString defaultUf2 =
            QStringLiteral("firmware/build/apps/pico_3d_mouse/pico_3d_mouse.uf2");
        m_fwWidget->setUf2FilePath(defaultUf2);

        rightCol->addWidget(m_calPanel);
        rightCol->addWidget(m_fwWidget);
        rightCol->addStretch(1);
        contentRow->addLayout(rightCol, 1);

        root->addLayout(contentRow, 1);
        setCentralWidget(central);

        statusBar()->showMessage(
            QStringLiteral("Ready — select RP2040 USB-C port (/dev/ttyACM0) and click Connect."));

        onRefreshPortsClicked();

        // ── Pure Event-Driven Signal/Slot Wiring (Zero Polling) ──────
        connect(m_refreshBtn,
                &QPushButton::clicked,
                this,
                &MainWindow::onRefreshPortsClicked);
        connect(m_connectBtn,
                &QPushButton::clicked,
                this,
                &MainWindow::onConnectToggleClicked);
        connect(m_resetTestBtn,
                &QPushButton::clicked,
                &m_controller,
                &ManufacturingController::requestResetManufacturingTest);

        connect(&m_serial,
                &ISerialTransport::connectionStateChanged,
                this,
                &MainWindow::onConnectionStateChanged);

        connect(&m_model,
                &DeviceTelemetryModel::telemetryUpdated,
                this,
                &MainWindow::onTelemetryUpdated);
        connect(&m_model,
                &DeviceTelemetryModel::calibrationUpdated,
                this,
                &MainWindow::onCalibrationUpdated);
        connect(&m_model,
                &DeviceTelemetryModel::manufacturingVerdictChanged,
                this,
                &MainWindow::onManufacturingVerdictChanged);
        connect(&m_model,
                &DeviceTelemetryModel::deviceMessageReceived,
                statusBar(),
                [this](const QString& msg) { statusBar()->showMessage(msg); });

        connect(&m_controller,
                &ManufacturingController::statusAnnouncement,
                statusBar(),
                [this](const QString& msg) { statusBar()->showMessage(msg); });

        // CalibrationPanelWidget -> ManufacturingController
        connect(m_calPanel,
                &CalibrationPanelWidget::zeroCenterRequested,
                &m_controller,
                &ManufacturingController::requestZeroCenter);
        connect(m_calPanel,
                &CalibrationPanelWidget::minMaxSweepStartRequested,
                &m_controller,
                &ManufacturingController::requestMinMaxSweepStart);
        connect(m_calPanel,
                &CalibrationPanelWidget::minMaxSweepEndRequested,
                &m_controller,
                &ManufacturingController::requestMinMaxSweepEnd);
        connect(m_calPanel,
                &CalibrationPanelWidget::saveCalibrationRequested,
                &m_controller,
                &ManufacturingController::requestSaveCalibration);
        connect(m_calPanel,
                &CalibrationPanelWidget::loadCalibrationRequested,
                &m_controller,
                &ManufacturingController::requestLoadCalibration);
        connect(m_calPanel,
                &CalibrationPanelWidget::resetCalibrationRequested,
                &m_controller,
                &ManufacturingController::requestResetCalibration);
        connect(m_calPanel,
                &CalibrationPanelWidget::deadzoneChanged,
                &m_controller,
                &ManufacturingController::requestSetDeadzone);
        connect(m_calPanel,
                &CalibrationPanelWidget::invertChanged,
                &m_controller,
                &ManufacturingController::requestSetInvert);

        // FirmwareUpdaterWidget <-> ManufacturingController / IFirmwareUpdater
        connect(m_fwWidget,
                &FirmwareUpdaterWidget::firmwareUpdateRequested,
                &m_controller,
                &ManufacturingController::requestButtonlessFirmwareUpdate);
        connect(&m_updater,
                &IFirmwareUpdater::updateStarted,
                m_fwWidget,
                [this](const QString&) { m_fwWidget->setUpdatingState(true); });
        connect(&m_updater,
                &IFirmwareUpdater::progressLog,
                m_fwWidget,
                &FirmwareUpdaterWidget::appendLogLine);
        connect(&m_updater,
                &IFirmwareUpdater::updateFinished,
                m_fwWidget,
                &FirmwareUpdaterWidget::setUpdateResult);
    }

    void MainWindow::onRefreshPortsClicked()
    {
        const QString current = m_portCombo->currentText();
        m_portCombo->clear();

        const QStringList ports = m_serial.availablePorts();
        if (ports.isEmpty())
        {
            m_portCombo->addItem(QStringLiteral("/dev/ttyACM0"));
        }
        else
        {
            m_portCombo->addItems(ports);
        }

        if (!current.isEmpty())
        {
            const int idx = m_portCombo->findText(current);
            if (idx >= 0)
            {
                m_portCombo->setCurrentIndex(idx);
            }
        }
    }

    void MainWindow::onConnectToggleClicked()
    {
        if (m_serial.isConnected())
        {
            m_controller.requestDisconnect();
        }
        else
        {
            m_controller.requestConnect(m_portCombo->currentText().trimmed());
        }
    }

    void MainWindow::onConnectionStateChanged(bool connected, const QString& /*portName*/)
    {
        m_connectBtn->setText(connected ? QStringLiteral("Disconnect")
                                        : QStringLiteral("Connect"));
        m_portCombo->setEnabled(!connected);
        m_refreshBtn->setEnabled(!connected);
    }

    void MainWindow::onTelemetryUpdated()
    {
        for (std::size_t i = 0; i < kUiAxisCount; ++i)
        {
            const auto& live    = m_model.axisLive(i);
            const auto  verdict = m_model.axisVerdict(i);

            m_gauges[i]->setReading(live.rawCounts,
                                    live.millivolts,
                                    live.deltaCounts,
                                    live.scaledOutput,
                                    live.minObservedRaw,
                                    live.maxObservedRaw);
            m_gauges[i]->setManufacturingCheckStatus(verdict.centerInSpec,
                                                     verdict.negativeTravelReached,
                                                     verdict.positiveTravelReached);
        }

        m_radar->setAxesOutput(m_model.axisLive(0).scaledOutput,
                               m_model.axisLive(1).scaledOutput,
                               m_model.axisLive(2).scaledOutput,
                               500);
    }

    void MainWindow::onCalibrationUpdated(const QString& eventName)
    {
        for (std::size_t i = 0; i < kUiAxisCount; ++i)
        {
            const auto& cal     = m_model.axisCalibration(i);
            const auto  verdict = m_model.axisVerdict(i);

            m_gauges[i]->setCalibrationMarkers(
                cal.centerCounts, cal.deadzoneCounts, cal.invert);
            m_gauges[i]->setManufacturingCheckStatus(verdict.centerInSpec,
                                                     verdict.negativeTravelReached,
                                                     verdict.positiveTravelReached);

            m_calPanel->setAxisCalibration(static_cast<int>(i),
                                           cal.centerCounts,
                                           cal.minCounts,
                                           cal.maxCounts,
                                           cal.deadzoneCounts,
                                           cal.invert);
        }

        m_calPanel->setStatusMessage(
            QStringLiteral("Calibration status: %1").arg(eventName));
    }

    void MainWindow::onManufacturingVerdictChanged(bool allPassed)
    {
        if (allPassed)
        {
            m_verdictBadge->setText(QStringLiteral("MFG TEST: PASS — ALL 3 AXES IN SPEC"));
            m_verdictBadge->setStyleSheet(
                QStringLiteral("QLabel { background-color: #065F46; color: #D1FAE5; font-weight: bold; "
                               "padding: 6px 12px; border-radius: 6px; }"));
        }
        else
        {
            m_verdictBadge->setText(
                QStringLiteral("MFG TEST: IN PROGRESS (SWEEP ALL 3 AXES)"));
            m_verdictBadge->setStyleSheet(
                QStringLiteral("QLabel { background-color: #374151; color: #F9FAFB; font-weight: bold; "
                               "padding: 6px 12px; border-radius: 6px; }"));
        }
    }
} // namespace p3d::ui
