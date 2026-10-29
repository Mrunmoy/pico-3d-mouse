#pragma once

#include <cstdint>

#include "hal/IHidTransport.hpp"

namespace p3d::platform::rp2040
{
    class Rp2040HidTransport final : public hal::IHidTransport
    {
    public:
        static constexpr std::uint8_t kReportIdTranslation = 1;
        static constexpr std::uint8_t kReportIdRotation    = 2;

        bool init() override;
        void task() override;
        bool sendMotionReport(const hal::MotionReport6Dof& report) override;

    private:
        hal::MotionReport6Dof m_lastSent{};
        bool                  m_firstReport = true;
    };
}
