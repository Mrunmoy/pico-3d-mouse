#pragma once

#include <array>
#include <cstdint>

#include "domain/AxisProcessor.hpp"
#include "drivers/IPotentiometerSensor.hpp"
#include "hal/IGpioOutput.hpp"
#include "hal/ISerialIo.hpp"
#include "hal/ITimeProvider.hpp"

namespace p3d::domain
{
    struct AppConfig
    {
        std::uint32_t startupSettleMs        = 100;
        std::uint16_t calibrationSweeps      = 64;
        std::uint32_t samplePeriodMs         = 50;
        std::uint32_t ledToggleIntervalTicks = 10;
        std::array<AxisConfig, drivers::kNumPotAxes> axisConfigs{};
    };

    struct TelemetryFrame
    {
        drivers::PotentiometerSnapshot                   raw{};
        std::array<AxisOutput, drivers::kNumPotAxes>     axes{};
    };

    class AppController
    {
    public:
        AppController(drivers::IPotentiometerSensor& pots,
                      hal::IGpioOutput&              statusLed,
                      hal::ISerialIo&                serial,
                      hal::ITimeProvider&            time,
                      const AppConfig&               cfg = AppConfig{});

        // Initializes peripherals, waits for rail settling, and calibrates resting zero.
        bool start();

        // Executes one control-loop iteration (serial command check, sensor read, filtering, telemetry, LED).
        bool tick();

        const TelemetryFrame& lastFrame() const { return m_lastFrame; }

    private:
        void emitCalibrationBanner();

        drivers::IPotentiometerSensor&                  m_pots;
        hal::IGpioOutput&                               m_statusLed;
        hal::ISerialIo&                                 m_serial;
        hal::ITimeProvider&                             m_time;
        AppConfig                                       m_cfg;
        std::array<AxisProcessor, drivers::kNumPotAxes> m_processors;
        TelemetryFrame                                  m_lastFrame{};
        std::uint32_t                                   m_tickCount = 0;
        bool                                            m_ledLevel  = true;
    };
}
