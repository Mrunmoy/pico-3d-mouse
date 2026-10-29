#pragma once

#include "hal/INvmStorage.hpp"
#include <gmock/gmock.h>

#include <cstring>
#include <vector>

namespace p3d::test
{
    class MockNvmStorage : public hal::INvmStorage
    {
    public:
        MOCK_METHOD(bool, read, (std::uint8_t* outData, std::size_t length), (const, override));
        MOCK_METHOD(bool, write, (const std::uint8_t* data, std::size_t length), (override));
        MOCK_METHOD(std::size_t, capacity, (), (const, override));

        void useBackingStore(std::size_t bytes)
        {
            m_store.assign(bytes, 0xFFU);
            ON_CALL(*this, capacity).WillByDefault([this]() { return m_store.size(); });
            ON_CALL(*this, read)
                .WillByDefault([this](std::uint8_t* outData, std::size_t length) {
                    if (outData == nullptr || length > m_store.size())
                    {
                        return false;
                    }
                    std::memcpy(outData, m_store.data(), length);
                    return true;
                });
            ON_CALL(*this, write)
                .WillByDefault([this](const std::uint8_t* data, std::size_t length) {
                    if (data == nullptr || length > m_store.size())
                    {
                        return false;
                    }
                    std::memcpy(m_store.data(), data, length);
                    return true;
                });
        }

        std::vector<std::uint8_t>& rawStore() { return m_store; }

    private:
        std::vector<std::uint8_t> m_store{};
    };
}
