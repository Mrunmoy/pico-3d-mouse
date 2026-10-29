#include "platform/rp2040/Rp2040HidTransport.hpp"

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
        void packVec3Le(std::array<std::uint8_t, 6>& buf,
                        std::int16_t                 x,
                        std::int16_t                 y,
                        std::int16_t                 z)
        {
            const auto ux = static_cast<std::uint16_t>(x);
            const auto uy = static_cast<std::uint16_t>(y);
            const auto uz = static_cast<std::uint16_t>(z);

            buf[0] = static_cast<std::uint8_t>(ux & 0xFFU);
            buf[1] = static_cast<std::uint8_t>((ux >> 8U) & 0xFFU);
            buf[2] = static_cast<std::uint8_t>(uy & 0xFFU);
            buf[3] = static_cast<std::uint8_t>((uy >> 8U) & 0xFFU);
            buf[4] = static_cast<std::uint8_t>(uz & 0xFFU);
            buf[5] = static_cast<std::uint8_t>((uz >> 8U) & 0xFFU);
        }
    }

    bool Rp2040HidTransport::init()
    {
        m_firstReport = true;
        m_sendRotNext = false;
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

        const bool transNonZero = (report.tx != 0) || (report.ty != 0) || (report.tz != 0);
        const bool rotNonZero   = (report.rx != 0) || (report.ry != 0) || (report.rz != 0);

        const bool transChanged = m_firstReport ||
                                  (report.tx != m_lastSent.tx) ||
                                  (report.ty != m_lastSent.ty) ||
                                  (report.tz != m_lastSent.tz);

        const bool rotChanged = m_firstReport ||
                                (report.rx != m_lastSent.rx) ||
                                (report.ry != m_lastSent.ry) ||
                                (report.rz != m_lastSent.rz);

        const bool needTrans = transNonZero || transChanged;
        const bool needRot   = rotNonZero || rotChanged;

        auto sendTrans = [&]() -> bool {
            std::array<std::uint8_t, 6> payload{};
            packVec3Le(payload, report.tx, report.ty, report.tz);
            if (tud_hid_report(kReportIdTranslation, payload.data(), payload.size()))
            {
                m_lastSent.tx = report.tx;
                m_lastSent.ty = report.ty;
                m_lastSent.tz = report.tz;
                return true;
            }
            return false;
        };

        auto sendRot = [&]() -> bool {
            std::array<std::uint8_t, 6> payload{};
            packVec3Le(payload, report.rx, report.ry, report.rz);
            if (tud_hid_report(kReportIdRotation, payload.data(), payload.size()))
            {
                m_lastSent.rx = report.rx;
                m_lastSent.ry = report.ry;
                m_lastSent.rz = report.rz;
                return true;
            }
            return false;
        };

        bool sentAny = false;
        if (needTrans && needRot)
        {
            if (m_sendRotNext)
            {
                sentAny = sendRot();
                m_sendRotNext = false;
            }
            else
            {
                sentAny = sendTrans();
                m_sendRotNext = true;
            }
        }
        else if (needTrans)
        {
            sentAny = sendTrans();
            m_sendRotNext = true;
        }
        else if (needRot)
        {
            sentAny = sendRot();
            m_sendRotNext = false;
        }

        if (sentAny)
        {
            m_firstReport = false;
        }

        return true;
    }
}
