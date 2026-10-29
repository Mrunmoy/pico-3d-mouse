#pragma once

#include <array>
#include <cstdint>

#include "domain/AxisProcessor.hpp"
#include "domain/CalibrationStore.hpp"
#include "domain/CommandParser.hpp"
#include "drivers/IPotentiometerSensor.hpp"
#include "hal/IBootControl.hpp"
#include "hal/IGpioOutput.hpp"
#include "hal/IHidTransport.hpp"
#include "hal/ISerialIo.hpp"
#include "hal/ITimeProvider.hpp"

namespace p3d::domain
{
    struct AppConfig
    {
        std::uint32_t startupSettleMs        = 100;
        std::uint16_t calibrationSweeps      = 64;
        std::uint32_t samplePeriodMs         = 20;  // 50 Hz control & HID update loop
        std::uint32_t ledToggleIntervalTicks = 25;
        std::array<AxisConfig, drivers::kNumPotAxes> axisConfigs{};
    };

    struct TelemetryFrame
    {
        drivers::PotentiometerSnapshot               raw{};
        std::array<AxisOutput, drivers::kNumPotAxes> axes{};
        hal::MotionReport6Dof                        hidReport{};
    };

    class AppController
    {
    public:
        AppController(drivers::IPotentiometerSensor& pots,
                      ICalibrationStore&             calStore,
                      hal::IHidTransport&            hid,
                      hal::IBootControl&             boot,
                      hal::IGpioOutput&              statusLed,
                      hal::ISerialIo&                serial,
                      hal::ITimeProvider&            time,
                      const AppConfig&               cfg = AppConfig{});

        // Initializes peripherals, loads Flash calibration (or auto-zeros), and reports status.
        bool start();

        // Executes one non-blocking control-loop iteration:
        // processes serial commands, reads sensors, updates min/max capture, sends 6-DOF HID, and streams telemetry.
        bool tick();

        const TelemetryFrame&  lastFrame() const { return m_lastFrame; }
        const CalibrationData& calibration() const { return m_calData; }
        bool                   isCapturingMinMax() const { return m_capturingMinMax; }

    private:
        bool handleCommand(const Command& cmd);
        void applyCalibrationToProcessors();
        void emitCalibrationStatus(std::string_view tag);

        drivers::IPotentiometerSensor&                  m_pots;
        ICalibrationStore&                              m_calStore;
        hal::IHidTransport&                             m_hid;
        hal::IBootControl&                              m_boot;
        hal::IGpioOutput&                               m_statusLed;
        hal::ISerialIo&                                 m_serial;
        hal::ITimeProvider&                             m_time;
        AppConfig                                       m_cfg;
        CalibrationData                                 m_calData{};
        CommandParser                                   m_cmdParser{};
        std::array<AxisProcessor, drivers::kNumPotAxes> m_processors;
        TelemetryFrame                                  m_lastFrame{};
        std::uint32_t                                   m_tickCount       = 0;
        bool                                            m_ledLevel        = true;
        bool                                            m_capturingMinMax = false;
    };
}
