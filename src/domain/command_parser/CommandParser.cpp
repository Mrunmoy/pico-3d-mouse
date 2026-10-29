#include "domain/CommandParser.hpp"

#include <charconv>
#include <system_error>

namespace p3d::domain
{
    namespace
    {
        constexpr std::size_t   kNumAxes       = 3;
        constexpr std::uint16_t kMaxDeadzone   = 2047;

        std::string_view trimSpaces(std::string_view sv)
        {
            while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t'))
            {
                sv.remove_prefix(1);
            }
            while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t'))
            {
                sv.remove_suffix(1);
            }
            return sv;
        }

        std::string_view nextToken(std::string_view& rest)
        {
            rest = trimSpaces(rest);
            if (rest.empty())
            {
                return {};
            }
            const std::size_t pos = rest.find(' ');
            if (pos == std::string_view::npos)
            {
                const std::string_view tok = rest;
                rest = {};
                return tok;
            }
            const std::string_view tok = rest.substr(0, pos);
            rest.remove_prefix(pos + 1);
            return tok;
        }

        bool parseU16(std::string_view tok, std::uint16_t& outVal)
        {
            if (tok.empty())
            {
                return false;
            }
            std::uint16_t val = 0;
            const auto res = std::from_chars(tok.data(), tok.data() + tok.size(), val);
            if (res.ec != std::errc{} || res.ptr != (tok.data() + tok.size()))
            {
                return false;
            }
            outVal = val;
            return true;
        }
    }

    std::optional<Command> CommandParser::feed(char c)
    {
        // Single-key interactive shortcut when line buffer is empty
        if (m_len == 0 && !m_overflow && (c == 'z' || c == 'Z'))
        {
            return Command{ CommandType::CalibrateCenter, 0, 0, false };
        }

        if (c == '\r' || c == '\n')
        {
            if (m_overflow)
            {
                reset();
                return std::nullopt;
            }
            if (m_len == 0)
            {
                return std::nullopt;
            }
            const std::string_view line(m_buf.data(), m_len);
            reset();
            return parseLine(line);
        }

        if (m_overflow)
        {
            return std::nullopt;
        }

        if (m_len >= m_buf.size())
        {
            m_overflow = true;
            m_len      = 0;
            return std::nullopt;
        }

        m_buf[m_len++] = c;
        return std::nullopt;
    }

    std::optional<Command> CommandParser::parseLine(std::string_view line)
    {
        std::string_view rest = trimSpaces(line);
        const std::string_view verb = nextToken(rest);
        if (verb.empty())
        {
            return std::nullopt;
        }

        if (verb == "CAL_CENTER" || verb == "ZERO" || verb == "z" || verb == "Z")
        {
            return Command{ CommandType::CalibrateCenter, 0, 0, false };
        }
        if (verb == "CAL_MINMAX_START" || verb == "M")
        {
            return Command{ CommandType::StartMinMaxCapture, 0, 0, false };
        }
        if (verb == "CAL_MINMAX_END" || verb == "E")
        {
            return Command{ CommandType::EndMinMaxCapture, 0, 0, false };
        }
        if (verb == "SAVE_CAL" || verb == "S")
        {
            return Command{ CommandType::SaveCalibration, 0, 0, false };
        }
        if (verb == "LOAD_CAL" || verb == "L")
        {
            return Command{ CommandType::LoadCalibration, 0, 0, false };
        }
        if (verb == "RESET_CAL" || verb == "F")
        {
            return Command{ CommandType::ResetDefaults, 0, 0, false };
        }
        if (verb == "GET_CAL" || verb == "G")
        {
            return Command{ CommandType::QueryCalibration, 0, 0, false };
        }
        if (verb == "BOOTSEL" || verb == "B")
        {
            return Command{ CommandType::EnterBootloader, 0, 0, false };
        }
        if (verb == "SET_DZ")
        {
            const std::string_view axisTok = nextToken(rest);
            const std::string_view dzTok   = nextToken(rest);
            std::uint16_t axis = 0;
            std::uint16_t dz   = 0;
            if (!parseU16(axisTok, axis) || !parseU16(dzTok, dz))
            {
                return std::nullopt;
            }
            if (axis >= kNumAxes || dz > kMaxDeadzone)
            {
                return std::nullopt;
            }
            return Command{ CommandType::SetDeadzone, static_cast<std::size_t>(axis), dz, false };
        }
        if (verb == "SET_INV")
        {
            const std::string_view axisTok = nextToken(rest);
            const std::string_view invTok  = nextToken(rest);
            std::uint16_t axis = 0;
            std::uint16_t inv  = 0;
            if (!parseU16(axisTok, axis) || !parseU16(invTok, inv))
            {
                return std::nullopt;
            }
            if (axis >= kNumAxes || inv > 1U)
            {
                return std::nullopt;
            }
            return Command{ CommandType::SetInvert, static_cast<std::size_t>(axis), 0, (inv == 1U) };
        }

        return std::nullopt;
    }

    void CommandParser::reset()
    {
        m_len      = 0;
        m_overflow = false;
    }
}
