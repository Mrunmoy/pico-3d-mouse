#include "platform/rp2040/Rp2040HidTransport.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "tusb.h"
#pragma GCC diagnostic pop

namespace p3d::platform::rp2040
{
    namespace
    {
        std::int16_t clampAxis(std::int16_t value)
        {
            return std::clamp(value,
                              static_cast<std::int16_t>(-Rp2040HidTransport::kMaxHidAxisValue),
                              Rp2040HidTransport::kMaxHidAxisValue);
        }

        void packInt16Le(std::uint8_t* dst, std::int16_t value, bool jiggleBit)
        {
            const std::int16_t clamped = clampAxis(value);
            auto               uval    = static_cast<std::uint16_t>(clamped);
            if (clamped != 0)
            {
                if (jiggleBit)
                {
                    uval |= 0x0001U;
                }
                else
                {
                    uval &= static_cast<std::uint16_t>(~0x0001U);
                }
            }
            dst[0] = static_cast<std::uint8_t>(uval & 0xFFU);
            dst[1] = static_cast<std::uint8_t>((uval >> 8U) & 0xFFU);
        }
    }

    bool Rp2040HidTransport::init()
    {
        m_zeroPacketsSent = 0U; // Send initial zero packet on startup
        m_jiggleBit       = false;
        return true;
    }

    void Rp2040HidTransport::task()
    {
        tud_task();
    }

    bool Rp2040HidTransport::sendMotionReport(const hal::MotionReport6Dof& report)
    {
        if (!tud_hid_ready())
        {
            return false;
        }

        const bool anyNonZero =
            (report.tx != 0) || (report.ty != 0) || (report.tz != 0) ||
            (report.rx != 0) || (report.ry != 0) || (report.rz != 0);

        if (!anyNonZero && m_zeroPacketsSent >= 3U)
        {
            return true;
        }

        std::array<std::uint8_t, 12> payload{};
        packInt16Le(&payload[0],  report.tx, m_jiggleBit);
        packInt16Le(&payload[2],  report.ty, m_jiggleBit);
        packInt16Le(&payload[4],  report.tz, m_jiggleBit);
        packInt16Le(&payload[6],  report.rx, m_jiggleBit);
        packInt16Le(&payload[8],  report.ry, m_jiggleBit);
        packInt16Le(&payload[10], report.rz, m_jiggleBit);

        if (tud_hid_report(kReportIdMotion6Dof, payload.data(), payload.size()))
        {
            m_jiggleBit = !m_jiggleBit;
            if (anyNonZero)
            {
                m_zeroPacketsSent = 0U;
            }
            else
            {
                ++m_zeroPacketsSent;
            }
        }

        return true;
    }
}
