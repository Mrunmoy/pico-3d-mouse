#include "domain/CalibrationStore.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace p3d::domain
{
    namespace
    {
        constexpr std::uint32_t kMagic         = 0x5033444DU;  // "P3DM"
        constexpr std::uint16_t kFormatVersion = 1U;
        constexpr std::uint16_t kMaxAdcCount   = 4095U;

        void writeU16Le(std::uint8_t* dst, std::uint16_t val)
        {
            dst[0] = static_cast<std::uint8_t>(val & 0xFFU);
            dst[1] = static_cast<std::uint8_t>((val >> 8U) & 0xFFU);
        }

        std::uint16_t readU16Le(const std::uint8_t* src)
        {
            return static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(src[0]) |
                static_cast<std::uint16_t>(static_cast<std::uint16_t>(src[1]) << 8U));
        }

        void writeU32Le(std::uint8_t* dst, std::uint32_t val)
        {
            dst[0] = static_cast<std::uint8_t>(val & 0xFFU);
            dst[1] = static_cast<std::uint8_t>((val >> 8U) & 0xFFU);
            dst[2] = static_cast<std::uint8_t>((val >> 16U) & 0xFFU);
            dst[3] = static_cast<std::uint8_t>((val >> 24U) & 0xFFU);
        }

        std::uint32_t readU32Le(const std::uint8_t* src)
        {
            return static_cast<std::uint32_t>(src[0]) |
                   (static_cast<std::uint32_t>(src[1]) << 8U) |
                   (static_cast<std::uint32_t>(src[2]) << 16U) |
                   (static_cast<std::uint32_t>(src[3]) << 24U);
        }

        // Standard IEEE 802.3 CRC-32 (bitwise, zero lookup table footprint).
        std::uint32_t computeCrc32(const std::uint8_t* data, std::size_t length)
        {
            std::uint32_t crc = 0xFFFFFFFFU;
            for (std::size_t i = 0; i < length; ++i)
            {
                crc ^= static_cast<std::uint32_t>(data[i]);
                for (int bit = 0; bit < 8; ++bit)
                {
                    if ((crc & 1U) != 0U)
                    {
                        crc = (crc >> 1U) ^ 0xEDB88320U;
                    }
                    else
                    {
                        crc >>= 1U;
                    }
                }
            }
            return ~crc;
        }
    }

    CalibrationStore::CalibrationStore(hal::INvmStorage& nvm)
        : m_nvm(nvm)
    {
    }

    bool CalibrationStore::isValidData(const CalibrationData& data)
    {
        for (std::size_t i = 0; i < kCalibrationAxes; ++i)
        {
            if (data.centerCounts[i] > kMaxAdcCount ||
                data.minCounts[i] > kMaxAdcCount ||
                data.maxCounts[i] > kMaxAdcCount)
            {
                return false;
            }
            if (data.minCounts[i] >= data.centerCounts[i] ||
                data.maxCounts[i] <= data.centerCounts[i])
            {
                return false;
            }
            if (data.deadzoneCounts[i] >= (kMaxAdcCount / 2U))
            {
                return false;
            }
        }
        return true;
    }

    bool CalibrationStore::save(const CalibrationData& data)
    {
        if (!isValidData(data))
        {
            return false;
        }

        // Layout (36 bytes total):
        //   [0..3]   : magic (u32)
        //   [4..5]   : version (u16)
        //   [6..11]  : centerCounts[0..2] (3 x u16)
        //   [12..17] : minCounts[0..2] (3 x u16)
        //   [18..23] : maxCounts[0..2] (3 x u16)
        //   [24..29] : deadzoneCounts[0..2] (3 x u16)
        //   [30]     : invert bitmask (bit 0..2)
        //   [31]     : reserved (0)
        //   [32..35] : crc32 over [0..31] (u32)
        std::array<std::uint8_t, kSerializedSize> buf{};
        writeU32Le(&buf[0], kMagic);
        writeU16Le(&buf[4], kFormatVersion);

        for (std::size_t i = 0; i < kCalibrationAxes; ++i)
        {
            writeU16Le(&buf[6U + (i * 2U)], data.centerCounts[i]);
            writeU16Le(&buf[12U + (i * 2U)], data.minCounts[i]);
            writeU16Le(&buf[18U + (i * 2U)], data.maxCounts[i]);
            writeU16Le(&buf[24U + (i * 2U)], data.deadzoneCounts[i]);
        }

        std::uint8_t invertMask = 0U;
        for (std::size_t i = 0; i < kCalibrationAxes; ++i)
        {
            if (data.invert[i])
            {
                invertMask = static_cast<std::uint8_t>(invertMask | (1U << i));
            }
        }
        buf[30] = invertMask;
        buf[31] = 0U;

        const std::uint32_t crc = computeCrc32(buf.data(), 32U);
        writeU32Le(&buf[32], crc);

        return m_nvm.write(buf.data(), buf.size());
    }

    bool CalibrationStore::load(CalibrationData& outData) const
    {
        std::array<std::uint8_t, kSerializedSize> buf{};
        if (!m_nvm.read(buf.data(), buf.size()))
        {
            return false;
        }

        if (readU32Le(&buf[0]) != kMagic || readU16Le(&buf[4]) != kFormatVersion)
        {
            return false;
        }

        const std::uint32_t expectedCrc = computeCrc32(buf.data(), 32U);
        const std::uint32_t storedCrc   = readU32Le(&buf[32]);
        if (expectedCrc != storedCrc)
        {
            return false;
        }

        CalibrationData candidate{};
        for (std::size_t i = 0; i < kCalibrationAxes; ++i)
        {
            candidate.centerCounts[i]   = readU16Le(&buf[6U + (i * 2U)]);
            candidate.minCounts[i]      = readU16Le(&buf[12U + (i * 2U)]);
            candidate.maxCounts[i]      = readU16Le(&buf[18U + (i * 2U)]);
            candidate.deadzoneCounts[i] = readU16Le(&buf[24U + (i * 2U)]);
            candidate.invert[i]         = ((buf[30] & (1U << i)) != 0U);
        }

        if (!isValidData(candidate))
        {
            return false;
        }

        outData = candidate;
        return true;
    }
}
