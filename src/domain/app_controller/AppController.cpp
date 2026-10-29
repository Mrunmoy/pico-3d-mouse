#include "domain/AppController.hpp"

#include <cstdio>

namespace p3d::domain
{
    AppController::AppController(drivers::IPotentiometerSensor& pots,
                                 hal::IGpioOutput&              statusLed,
                                 hal::ISerialIo&                serial,
                                 hal::ITimeProvider&            time,
                                 const AppConfig&               cfg)
        : m_pots(pots)
        , m_statusLed(statusLed)
        , m_serial(serial)
        , m_time(time)
        , m_cfg(cfg)
        , m_processors{
              AxisProcessor(cfg.axisConfigs[0]),
              AxisProcessor(cfg.axisConfigs[1]),
              AxisProcessor(cfg.axisConfigs[2]),
          }
    {
    }

    bool AppController::start()
    {
        if (!m_serial.init())
        {
            return false;
        }
        if (!m_statusLed.init())
        {
            return false;
        }
        m_ledLevel = true;
        if (!m_statusLed.write(m_ledLevel))
        {
            return false;
        }
        if (!m_pots.init())
        {
            return false;
        }

        m_time.delayMs(m_cfg.startupSettleMs);

        if (!m_pots.calibrateCenter(m_cfg.calibrationSweeps))
        {
            return false;
        }

        for (auto& proc : m_processors)
        {
            proc.reset();
        }

        emitCalibrationBanner();
        return true;
    }

    bool AppController::tick()
    {
        const int cmd = m_serial.readCharNonBlocking();
        if (cmd == 'z' || cmd == 'Z')
        {
            if (!m_pots.calibrateCenter(m_cfg.calibrationSweeps))
            {
                return false;
            }
            for (auto& proc : m_processors)
            {
                proc.reset();
            }
            emitCalibrationBanner();
        }

        if (!m_pots.readSnapshot(m_lastFrame.raw))
        {
            return false;
        }

        for (std::size_t i = 0; i < drivers::kNumPotAxes; ++i)
        {
            m_lastFrame.axes[i] = m_processors[i].process(m_lastFrame.raw.rawDelta[i]);
        }

        char line[192]{};
        std::snprintf(
            line,
            sizeof(line),
            "FWD_BCK(ADC0)[raw=%4u %4umV d=%+5d out=%+4d]  "
            "LFT_RGT(ADC1)[raw=%4u %4umV d=%+5d out=%+4d]  "
            "ROT(ADC2)[raw=%4u %4umV d=%+5d out=%+4d]",
            static_cast<unsigned>(m_lastFrame.raw.rawCounts[0]),
            static_cast<unsigned>(m_lastFrame.raw.millivolts[0]),
            static_cast<int>(m_lastFrame.axes[0].conditionedDelta),
            static_cast<int>(m_lastFrame.axes[0].scaled),
            static_cast<unsigned>(m_lastFrame.raw.rawCounts[1]),
            static_cast<unsigned>(m_lastFrame.raw.millivolts[1]),
            static_cast<int>(m_lastFrame.axes[1].conditionedDelta),
            static_cast<int>(m_lastFrame.axes[1].scaled),
            static_cast<unsigned>(m_lastFrame.raw.rawCounts[2]),
            static_cast<unsigned>(m_lastFrame.raw.millivolts[2]),
            static_cast<int>(m_lastFrame.axes[2].conditionedDelta),
            static_cast<int>(m_lastFrame.axes[2].scaled));
        m_serial.writeLine(line);

        ++m_tickCount;
        const std::uint32_t toggleInterval =
            (m_cfg.ledToggleIntervalTicks == 0U) ? 1U : m_cfg.ledToggleIntervalTicks;
        if ((m_tickCount % toggleInterval) == 0U)
        {
            m_ledLevel = !m_ledLevel;
            (void)m_statusLed.write(m_ledLevel);
        }

        m_time.delayMs(m_cfg.samplePeriodMs);
        return true;
    }

    void AppController::emitCalibrationBanner()
    {
        const auto& c = m_pots.centerCounts();
        char banner[160]{};
        std::snprintf(
            banner,
            sizeof(banner),
            "[pico-3d-mouse] Center calibrated: "
            "FWD_BCK(ADC0/GP26)=%4u  LFT_RGT(ADC1/GP27)=%4u  ROT(ADC2/GP28)=%4u  (press 'z' to re-zero)",
            static_cast<unsigned>(c[0]),
            static_cast<unsigned>(c[1]),
            static_cast<unsigned>(c[2]));
        m_serial.writeLine(banner);
    }
}
