#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "PicoSdkStubState.hpp"

inline constexpr std::uint32_t FLASH_PAGE_SIZE       = 256U;
inline constexpr std::uint32_t FLASH_SECTOR_SIZE     = 4096U;
inline constexpr std::uint32_t PICO_FLASH_SIZE_BYTES = 2U * 1024U * 1024U;

inline void flash_range_erase(std::uint32_t /*flash_offs*/, std::size_t /*count*/)
{
    ++p3d::test::sdk_stub::state().flashEraseCount;
    p3d::test::sdk_stub::state().flashSector.fill(0xFFU);
}

inline void flash_range_program(std::uint32_t /*flash_offs*/,
                                const std::uint8_t* data,
                                std::size_t         count)
{
    ++p3d::test::sdk_stub::state().flashProgramCount;
    const std::size_t n = std::min(count, p3d::test::sdk_stub::state().flashSector.size());
    std::memcpy(p3d::test::sdk_stub::state().flashSector.data(), data, n);
}
