#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace p3d::test::sdk_stub
{
    struct State
    {
        bool                       adcInitialized      = false;
        std::vector<std::uint32_t> adcInitializedGpios{};
        std::vector<std::uint32_t> pullsDisabledGpios{};
        std::uint32_t              selectedAdcInput    = 0xFF;
        std::uint16_t              nextAdcReadValue    = 0;

        std::uint32_t              lastGpioInitPin     = 0xFF;
        bool                       lastGpioDirOut      = false;
        std::uint32_t              lastGpioPutPin      = 0xFF;
        bool                       lastGpioPutLevel    = false;

        std::uint32_t              lastSleepMs         = 0;
        std::uint32_t              lastSleepUs         = 0;
        std::uint64_t              currentTimestampUs  = 0;

        bool                       stdioInitResult     = true;
        int                        nextGetcharResult   = -1;
    };

    inline State& state()
    {
        static State s{};
        return s;
    }

    inline void reset()
    {
        state() = State{};
    }
}
