#include "platform/rp2040/Rp2040UsbSerialIo.hpp"

#include <cstdio>

#include "pico/stdlib.h"

namespace p3d::platform::rp2040
{
    bool Rp2040UsbSerialIo::init()
    {
        return stdio_init_all();
    }

    int Rp2040UsbSerialIo::readCharNonBlocking()
    {
        const int ch = getchar_timeout_us(0);
        if (ch == PICO_ERROR_TIMEOUT)
        {
            return -1;
        }
        return ch;
    }

    void Rp2040UsbSerialIo::writeLine(const char* str)
    {
        std::printf("%s\r\n", str);
    }
}
