#include "domain/AppController.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace p3d::domain
{
    namespace
    {
        class FixedLineBuffer
        {
        public:
            void append(std::string_view sv)
            {
                for (const char c : sv)
                {
                    if (m_len < m_buf.size())
                    {
                        m_buf[m_len++] = c;
                    }
                }
            }

            void appendUnsigned(std::uint16_t value, std::size_t width = 0)
            {
                std::array<char, 16> tmp{};
                const auto res = std::to_chars(tmp.data(), tmp.data() + tmp.size(), value);
                const auto digits = static_cast<std::size_t>(res.ptr - tmp.data());
                for (std::size_t pad = digits; pad < width; ++pad)
                {
                    append(" ");
                }
                append(std::string_view(tmp.data(), digits));
            }

            void appendSigned(std::int16_t value, std::size_t width = 5)
            {
                const char sign = (value < 0) ? '-' : '+';
                const std::uint16_t mag = (value < 0)
                    ? static_cast<std::uint16_t>(-static_cast<std::int32_t>(value))
                    : static_cast<std::uint16_t>(value);

                std::array<char, 16> tmp{};
                const auto res = std::to_chars(tmp.data(), tmp.data() + tmp.size(), mag);
                const auto digits = static_cast<std::size_t>(res.ptr - tmp.data());
                const std::size_t totalChars = digits + 1U;

                for (std::size_t pad = totalChars; pad < width; ++pad)
                {
                    append(" ");
                }
                append(std::string_view(&sign, 1U));
                append(std::string_view(tmp.data(), digits));
            }

            void appendAxis(std::string_view name,
                            std::uint16_t    raw,
                            std::uint16_t    mv,
                            std::int16_t     delta,
                            std::int16_t     scaled)
            {
                append(name);
                append("[raw=");
                appendUnsigned(raw, 4);
                append(" ");
                appendUnsigned(mv, 4);
                append("mV d=");
                appendSigned(delta, 5);
                append(" out=");
                appendSigned(scaled, 4);
                append("]");
            }

            std::string_view view() const
            {
                return std::string_view(m_buf.data(), m_len);
            }

        private:
            std::array<char, 240> m_buf{};
            std::size_t           m_len = 0;
        };
    }

    AppController::AppController(drivers::IPotentiometerSensor& pots,
                                 ICalibrationStore&             calStore,
                                 hal::IHidTransport&            hid,
                                 hal::IBootControl&             boot,
                                 hal::IGpioOutput&              statusLed,
                                 hal::ISerialIo&                serial,
                                 hal::ITimeProvider&            time,
                                 const AppConfig&               cfg)
        : m_pots(pots)
        , m_calStore(calStore)
        , m_hid(hid)
        , m_boot(boot)
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
        for (std::size_t i = 0; i < drivers::kNumPotAxes; ++i)
        {
            m_calData.deadzoneCounts[i] = cfg.axisConfigs[i].deadzoneCounts;
            m_calData.invert[i]         = cfg.axisConfigs[i].invert;
        }
    }

    bool AppController::start()
    {
        if (!m_serial.init())
        {
            return false;
        }
        if (!m_hid.init())
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

        CalibrationData loaded{};
        if (m_calStore.load(loaded))
        {
            m_calData = loaded;
            applyCalibrationToProcessors();
            emitCalibrationStatus("LOADED");
        }
        else
        {
            if (!m_pots.calibrateCenter(m_cfg.calibrationSweeps))
            {
                return false;
            }
            m_calData.centerCounts = m_pots.centerCounts();
            applyCalibrationToProcessors();
            emitCalibrationStatus("CENTER_CALIBRATED");
        }

        return true;
    }

    bool AppController::tick()
    {
        m_hid.task();

        while (const std::optional<char> ch = m_serial.readCharNonBlocking())
        {
            if (const std::optional<Command> cmd = m_cmdParser.feed(*ch))
            {
                if (!handleCommand(*cmd))
                {
                    return false;
                }
            }
        }

        if (!m_pots.readSnapshot(m_lastFrame.raw))
        {
            return false;
        }

        for (std::size_t i = 0; i < drivers::kNumPotAxes; ++i)
        {
            const std::uint16_t raw = m_lastFrame.raw.rawCounts[i];

            if (m_capturingMinMax)
            {
                m_calData.minCounts[i] = std::min(m_calData.minCounts[i], raw);
                m_calData.maxCounts[i] = std::max(m_calData.maxCounts[i], raw);
            }

            const std::int16_t delta = static_cast<std::int16_t>(
                static_cast<std::int32_t>(raw) -
                static_cast<std::int32_t>(m_calData.centerCounts[i]));

            m_lastFrame.raw.rawDelta[i] = delta;
            m_lastFrame.axes[i]         = m_processors[i].process(delta);
        }

        // Map conditioned 3-axis joystick to 6-DOF report (Pan/Zoom + Twist):
        //   ADC0 (Index 0) = Forward/Backward -> ty (Forward/Backward Zoom)
        //   ADC1 (Index 1) = Left/Right       -> tx (Left/Right Pan)
        //   ADC2 (Index 2) = Rotation (Twist) -> rz (Yaw Twist)
        m_lastFrame.hidReport = hal::MotionReport6Dof{};
        m_lastFrame.hidReport.ty = m_lastFrame.axes[0].scaled;
        m_lastFrame.hidReport.tx = m_lastFrame.axes[1].scaled;
        m_lastFrame.hidReport.rz = m_lastFrame.axes[2].scaled;

        (void)m_hid.sendMotionReport(m_lastFrame.hidReport);

        FixedLineBuffer line{};
        line.appendAxis("FWD_BCK(ADC0)",
                        m_lastFrame.raw.rawCounts[0],
                        m_lastFrame.raw.millivolts[0],
                        m_lastFrame.axes[0].conditionedDelta,
                        m_lastFrame.axes[0].scaled);
        line.append("  ");
        line.appendAxis("LFT_RGT(ADC1)",
                        m_lastFrame.raw.rawCounts[1],
                        m_lastFrame.raw.millivolts[1],
                        m_lastFrame.axes[1].conditionedDelta,
                        m_lastFrame.axes[1].scaled);
        line.append("  ");
        line.appendAxis("ROT(ADC2)",
                        m_lastFrame.raw.rawCounts[2],
                        m_lastFrame.raw.millivolts[2],
                        m_lastFrame.axes[2].conditionedDelta,
                        m_lastFrame.axes[2].scaled);

        m_serial.writeLine(line.view());

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

    bool AppController::handleCommand(const Command& cmd)
    {
        switch (cmd.type)
        {
            case CommandType::CalibrateCenter:
            {
                if (!m_pots.calibrateCenter(m_cfg.calibrationSweeps))
                {
                    return false;
                }
                m_calData.centerCounts = m_pots.centerCounts();
                applyCalibrationToProcessors();
                emitCalibrationStatus("CENTER_CALIBRATED");
                return true;
            }
            case CommandType::StartMinMaxCapture:
            {
                m_capturingMinMax = true;
                for (std::size_t i = 0; i < drivers::kNumPotAxes; ++i)
                {
                    const std::uint16_t c = m_calData.centerCounts[i];
                    m_calData.minCounts[i] = (c > 0U) ? static_cast<std::uint16_t>(c - 1U) : 0U;
                    m_calData.maxCounts[i] = (c < 4095U) ? static_cast<std::uint16_t>(c + 1U) : 4095U;
                }
                m_serial.writeLine("@CAL MINMAX_CAPTURE_STARTED");
                return true;
            }
            case CommandType::EndMinMaxCapture:
            {
                m_capturingMinMax = false;
                applyCalibrationToProcessors();
                emitCalibrationStatus("MINMAX_CAPTURED");
                return true;
            }
            case CommandType::SetDeadzone:
            {
                if (cmd.axisIndex < drivers::kNumPotAxes)
                {
                    m_calData.deadzoneCounts[cmd.axisIndex] = cmd.uintParam;
                    applyCalibrationToProcessors();
                    emitCalibrationStatus("DEADZONE_UPDATED");
                }
                return true;
            }
            case CommandType::SetInvert:
            {
                if (cmd.axisIndex < drivers::kNumPotAxes)
                {
                    m_calData.invert[cmd.axisIndex] = cmd.boolParam;
                    applyCalibrationToProcessors();
                    emitCalibrationStatus("INVERT_UPDATED");
                }
                return true;
            }
            case CommandType::SaveCalibration:
            {
                if (m_calStore.save(m_calData))
                {
                    emitCalibrationStatus("SAVED_OK");
                }
                else
                {
                    m_serial.writeLine("@CAL SAVE_ERR");
                }
                return true;
            }
            case CommandType::LoadCalibration:
            {
                CalibrationData loaded{};
                if (m_calStore.load(loaded))
                {
                    m_calData = loaded;
                    applyCalibrationToProcessors();
                    emitCalibrationStatus("LOADED");
                }
                else
                {
                    m_serial.writeLine("@CAL LOAD_ERR");
                }
                return true;
            }
            case CommandType::ResetDefaults:
            {
                m_calData = CalibrationData{};
                if (!m_pots.calibrateCenter(m_cfg.calibrationSweeps))
                {
                    return false;
                }
                m_calData.centerCounts = m_pots.centerCounts();
                applyCalibrationToProcessors();
                emitCalibrationStatus("RESET_DEFAULTS");
                return true;
            }
            case CommandType::QueryCalibration:
            {
                emitCalibrationStatus("STATUS");
                return true;
            }
            case CommandType::EnterBootloader:
            {
                m_serial.writeLine("@BOOT ENTERING_BOOTSEL");
                m_boot.enterBootloader();
                return true;
            }
        }
        return true;
    }

    void AppController::applyCalibrationToProcessors()
    {
        for (std::size_t i = 0; i < drivers::kNumPotAxes; ++i)
        {
            AxisConfig cfg = m_cfg.axisConfigs[i];
            cfg.deadzoneCounts = m_calData.deadzoneCounts[i];
            cfg.invert         = m_calData.invert[i];

            const std::uint16_t posSpan = (m_calData.maxCounts[i] > m_calData.centerCounts[i])
                ? static_cast<std::uint16_t>(m_calData.maxCounts[i] - m_calData.centerCounts[i])
                : 1U;
            const std::uint16_t negSpan = (m_calData.centerCounts[i] > m_calData.minCounts[i])
                ? static_cast<std::uint16_t>(m_calData.centerCounts[i] - m_calData.minCounts[i])
                : 1U;
            const std::uint16_t maxSpan = std::max(posSpan, negSpan);
            cfg.maxDeflectionCounts = (maxSpan > cfg.deadzoneCounts)
                ? static_cast<std::uint16_t>(maxSpan - cfg.deadzoneCounts)
                : 1U;

            m_processors[i].setConfig(cfg);
            m_processors[i].reset();
        }
    }

    void AppController::emitCalibrationStatus(std::string_view tag)
    {
        FixedLineBuffer line{};
        line.append("@CAL ");
        line.append(tag);
        for (std::size_t i = 0; i < drivers::kNumPotAxes; ++i)
        {
            line.append(" CH");
            line.appendUnsigned(static_cast<std::uint16_t>(i));
            line.append("[c=");
            line.appendUnsigned(m_calData.centerCounts[i]);
            line.append(" min=");
            line.appendUnsigned(m_calData.minCounts[i]);
            line.append(" max=");
            line.appendUnsigned(m_calData.maxCounts[i]);
            line.append(" dz=");
            line.appendUnsigned(m_calData.deadzoneCounts[i]);
            line.append(" inv=");
            line.appendUnsigned(m_calData.invert[i] ? 1U : 0U);
            line.append("]");
        }
        m_serial.writeLine(line.view());
    }
}
