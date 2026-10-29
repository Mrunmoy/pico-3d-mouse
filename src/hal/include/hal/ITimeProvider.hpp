#pragma once

#include <cstdint>

namespace p3d::hal
{
    // Pure hardware abstraction for monotonic timestamps and blocking delays.
    class ITimeProvider
    {
    public:
        virtual ~ITimeProvider() = default;

        virtual void delayMs(std::uint32_t ms) = 0;
        virtual void delayUs(std::uint32_t us) = 0;
        virtual std::uint64_t getTimestampUs() const = 0;
    };
}
