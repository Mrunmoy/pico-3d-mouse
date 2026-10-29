#pragma once

#include <cstdint>

#include "hal/IHidTransport.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040HidTransport final : public hal::IHidTransport
    {
    public:
        static constexpr std::uint8_t kReportIdMotion6Dof  = 1;
        static constexpr std::uint8_t kReportIdTranslation = 1;
        static constexpr std::uint8_t kReportIdRotation    = 2;
        static constexpr std::int16_t kMaxHidAxisValue     = 350;

        bool init() override;
        void task() override;
        bool sendMotionReport(const hal::MotionReport6Dof& report) override;

    private:
        std::uint8_t m_zeroPacketsSent = 3U;
        bool         m_jiggleBit       = false;
    };
}
