#pragma once

#include <cstdint>

namespace p3d::hal
{
    // 6-DOF motion report for a 3D Mouse / Multi-Axis Controller.
    // Values are signed 16-bit logical deflections (typically in [-500, +500]).
    struct MotionReport6Dof
    {
        std::int16_t tx = 0;  // Left (-) / Right (+)
        std::int16_t ty = 0;  // Backward (-) / Forward (+)
        std::int16_t tz = 0;  // Down (-) / Up (+)
        std::int16_t rx = 0;  // Pitch
        std::int16_t ry = 0;  // Roll
        std::int16_t rz = 0;  // Rotation / Yaw twist
    };

    class IHidTransport
    {
    public:
        virtual ~IHidTransport() = default;

        virtual bool init() = 0;
        virtual void task() = 0;
        virtual bool sendMotionReport(const MotionReport6Dof& report) = 0;
    };
}
