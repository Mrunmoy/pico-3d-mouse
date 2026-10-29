#include "platform/rp2040/Rp2040UsbSerialIo.hpp"

#include <cstdio>

#include "pico/stdlib.h"

namespace p3d::platform::rp2040
{
    bool Rp2040UsbSerialIo::init()
    {
        return stdio_init_all();
    }

    std::optional<char> Rp2040UsbSerialIo::readCharNonBlocking()
    {
        const int ch = getchar_timeout_us(0);
        if (ch == PICO_ERROR_TIMEOUT)
        {
            return std::nullopt;
        }
        return static_cast<char>(ch);
    }

    void Rp2040UsbSerialIo::writeLine(std::string_view line)
    {
        if (!line.empty())
        {
            (void)std::fwrite(line.data(), 1U, line.size(), stdout);
        }
        (void)std::fwrite("\r\n", 1U, 2U, stdout);
        (void)std::fflush(stdout);
    }
}
