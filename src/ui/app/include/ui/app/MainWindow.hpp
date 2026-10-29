#pragma once

#include "ui/controllers/ManufacturingController.hpp"
#include "ui/interfaces/IFirmwareUpdater.hpp"
#include "ui/interfaces/ISerialTransport.hpp"
#include "ui/models/DeviceTelemetryModel.hpp"
#include "ui/widgets/AxisGaugeWidget.hpp"
#include "ui/widgets/CalibrationPanelWidget.hpp"
#include "ui/widgets/FirmwareUpdaterWidget.hpp"
#include "ui/widgets/GimbalRadarWidget.hpp"
#include "ui/widgets/ManufacturingWizardWidget.hpp"

#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <array>

namespace p3d::ui
{
    class MainWindow final : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(ISerialTransport&        serialTransport,
                   IFirmwareUpdater&        firmwareUpdater,
                   DeviceTelemetryModel&    telemetryModel,
                   ManufacturingController& controller,
                   QWidget*                 parent = nullptr);
        ~MainWindow() override = default;

        GimbalRadarWidget*         gimbalRadar() const { return m_radar; }
        AxisGaugeWidget*           axisGauge(std::size_t axis) const { return m_gauges.at(axis); }
        ManufacturingWizardWidget* manufacturingWizard() const { return m_wizard; }
        CalibrationPanelWidget*    calibrationPanel() const { return m_calPanel; }
        FirmwareUpdaterWidget*     firmwareUpdaterWidget() const { return m_fwWidget; }
        QLabel*                    overallVerdictBadge() const { return m_verdictBadge; }

    private slots:
        void onRefreshPortsClicked();
        void onConnectToggleClicked();
        void onConnectionStateChanged(bool connected, const QString& portName);
        void onTelemetryUpdated();
        void onCalibrationUpdated(const QString& eventName);
        void onManufacturingVerdictChanged(bool allPassed);

    private:
        ISerialTransport&        m_serial;
        IFirmwareUpdater&        m_updater;
        DeviceTelemetryModel&    m_model;
        ManufacturingController& m_controller;

        QComboBox*                      m_portCombo{ nullptr };
        QPushButton*                    m_refreshBtn{ nullptr };
        QPushButton*                    m_connectBtn{ nullptr };
        QPushButton*                    m_resetTestBtn{ nullptr };
        QLabel*                         m_verdictBadge{ nullptr };
        std::array<AxisGaugeWidget*, 3> m_gauges{};
        GimbalRadarWidget*              m_radar{ nullptr };
        QTabWidget*                     m_rightTabs{ nullptr };
        ManufacturingWizardWidget*      m_wizard{ nullptr };
        CalibrationPanelWidget*         m_calPanel{ nullptr };
        FirmwareUpdaterWidget*          m_fwWidget{ nullptr };
    };
} // namespace p3d::ui
