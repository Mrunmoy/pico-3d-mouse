#pragma once

#include <optional>
#include <string_view>

namespace p3d::hal
{
    // Pure C++17 hardware abstraction for non-blocking console input and line output.
    class ISerialIo
    {
    public:
        virtual ~ISerialIo() = default;

        virtual bool init() = 0;

        // Returns the next received character if available, or std::nullopt on timeout/empty FIFO.
        virtual std::optional<char> readCharNonBlocking() = 0;

        // Writes a single line of text to the serial stream.
        virtual void writeLine(std::string_view line) = 0;
    };
}
