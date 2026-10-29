#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace p3d::domain
{
    enum class CommandType : std::uint8_t
    {
        CalibrateCenter,
        StartMinMaxCapture,
        EndMinMaxCapture,
        SetDeadzone,
        SetInvert,
        SaveCalibration,
        LoadCalibration,
        ResetDefaults,
        QueryCalibration,
        EnterBootloader
    };

    struct Command
    {
        CommandType   type       = CommandType::CalibrateCenter;
        std::size_t   axisIndex  = 0;
        std::uint16_t uintParam  = 0;
        bool          boolParam  = false;
    };

    // Zero-heap, non-blocking stream parser for USB-CDC serial commands.
    class CommandParser
    {
    public:
        static constexpr std::size_t kMaxLineLength = 64;

        // Feeds a single incoming serial byte; returns a parsed Command when complete.
        std::optional<Command> feed(char c);

        // Parses a complete command line directly.
        static std::optional<Command> parseLine(std::string_view line);

        void reset();

    private:
        std::array<char, kMaxLineLength> m_buf{};
        std::size_t                      m_len      = 0;
        bool                             m_overflow = false;
    };
}
