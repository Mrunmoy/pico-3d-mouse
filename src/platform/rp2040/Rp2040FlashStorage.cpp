#include "platform/rp2040/Rp2040FlashStorage.hpp"

#include <array>
#include <cstring>

#include "hardware/flash.h"
#include "hardware/sync.h"

namespace p3d::platform::rp2040
{
    namespace
    {
        constexpr std::uint32_t kFlashTargetOffset = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
    }

    bool Rp2040FlashStorage::read(std::uint8_t* outData, std::size_t length) const
    {
        if (outData == nullptr || length == 0U || length > capacity())
        {
            return false;
        }

#if defined(PICO_ON_DEVICE)
        const auto* src = reinterpret_cast<const std::uint8_t*>(XIP_BASE + kFlashTargetOffset);
#else
        const auto* src = p3d::test::sdk_stub::state().flashSector.data();
#endif
        std::memcpy(outData, src, length);
        return true;
    }

    bool Rp2040FlashStorage::write(const std::uint8_t* data, std::size_t length)
    {
        if (data == nullptr || length == 0U || length > capacity())
        {
            return false;
        }

        std::array<std::uint8_t, FLASH_PAGE_SIZE> pageBuf{};
        pageBuf.fill(0xFFU);
        std::memcpy(pageBuf.data(), data, length);

        const std::uint32_t ints = save_and_disable_interrupts();
        flash_range_erase(kFlashTargetOffset, FLASH_SECTOR_SIZE);
        flash_range_program(kFlashTargetOffset, pageBuf.data(), FLASH_PAGE_SIZE);
        restore_interrupts(ints);

        return true;
    }

    std::size_t Rp2040FlashStorage::capacity() const
    {
        return FLASH_PAGE_SIZE;
    }
}
