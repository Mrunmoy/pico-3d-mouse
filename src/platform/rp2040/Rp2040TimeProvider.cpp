#include "platform/rp2040/Rp2040TimeProvider.hpp"

#include "pico/stdlib.h"
#include "hardware/timer.h"

namespace p3d::platform::rp2040
{
    void Rp2040TimeProvider::delayMs(std::uint32_t ms)
    {
        sleep_ms(ms);
    }

    void Rp2040TimeProvider::delayUs(std::uint32_t us)
    {
        sleep_us(us);
    }

    std::uint64_t Rp2040TimeProvider::getTimestampUs() const
    {
        return time_us_64();
    }
}
