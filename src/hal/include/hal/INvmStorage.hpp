#pragma once

#include <cstddef>
#include <cstdint>

namespace p3d::hal
{
    // Pure hardware abstraction for non-volatile memory persistence (e.g. QSPI flash sector).
    class INvmStorage
    {
    public:
        virtual ~INvmStorage() = default;

        virtual bool read(std::uint8_t* outData, std::size_t length) const = 0;
        virtual bool write(const std::uint8_t* data, std::size_t length) = 0;
        virtual std::size_t capacity() const = 0;
    };
}
