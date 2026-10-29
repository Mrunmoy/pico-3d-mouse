#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace p3d::test::sdk_stub
{
    inline constexpr std::size_t kStubFlashSectorSize = 4096;

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

        bool                       bootloaderEntered   = false;
        std::uint32_t              flashEraseCount     = 0;
        std::uint32_t              flashProgramCount   = 0;
        std::array<std::uint8_t, kStubFlashSectorSize> flashSector{};

        bool                       hidReady            = true;
        std::uint32_t              hidReportCount      = 0;
        std::uint8_t               lastHidReportId     = 0;

        State()
        {
            flashSector.fill(0xFFU);
        }
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
