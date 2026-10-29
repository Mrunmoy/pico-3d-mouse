#pragma once

namespace p3d::hal
{
    // Pure hardware abstraction for non-blocking console input and telemetry output.
    class ISerialIo
    {
    public:
        virtual ~ISerialIo() = default;

        virtual bool init() = 0;

        // Returns the next received character (0..255), or -1 if no character is available.
        virtual int readCharNonBlocking() = 0;

        // Writes a null-terminated string to the serial stream.
        virtual void writeLine(const char* str) = 0;
    };
}
