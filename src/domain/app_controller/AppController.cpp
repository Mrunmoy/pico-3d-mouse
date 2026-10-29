#include "domain/AppController.hpp"

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

            void appendUnsigned(std::uint16_t value, std::size_t width = 4)
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
            std::array<char, 224> m_buf{};
            std::size_t           m_len = 0;
        };
    }

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
        const std::optional<char> cmd = m_serial.readCharNonBlocking();
        if (cmd.has_value() && (*cmd == 'z' || *cmd == 'Z'))
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

    void AppController::emitCalibrationBanner()
    {
        const auto& c = m_pots.centerCounts();
        FixedLineBuffer banner{};
        banner.append("[pico-3d-mouse] Center calibrated: FWD_BCK(ADC0/GP26)=");
        banner.appendUnsigned(c[0], 4);
        banner.append("  LFT_RGT(ADC1/GP27)=");
        banner.appendUnsigned(c[1], 4);
        banner.append("  ROT(ADC2/GP28)=");
        banner.appendUnsigned(c[2], 4);
        banner.append("  (press 'z' to re-zero)");
        m_serial.writeLine(banner.view());
    }
}
