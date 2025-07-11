#pragma once

#include "muon/core/assert.hpp"
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace muon {

    template<typename T>
    class ArenaPtr {
    public:
        ArenaPtr(T *ptr) : m_ptr{ptr} {};
        ~ArenaPtr() { m_ptr->~T(); }

    public:
        [[nodiscard]] auto Get() const -> T * { return m_ptr; }
        [[nodiscard]] auto operator->() -> T * { return m_ptr; }
        [[nodiscard]] auto operator*() -> T & { return *m_ptr; }

    private:
        T *m_ptr{nullptr};
    };

    class ArenaAllocator {
    public:
        ArenaAllocator(size_t size) : m_size{size} {
            m_data = static_cast<uint8_t *>(std::malloc(size));
        }

        ~ArenaAllocator() { delete m_data; }

        template<typename T, typename ...Args>
        [[nodiscard]] auto Allocate(Args &&...args) -> ArenaPtr<T> {
            MU_CORE_ASSERT((m_offset + sizeof(T)) < m_size, "there is not enough room to allocate");
            m_offset += sizeof(T);
            return ArenaPtr<T>(new(m_data + m_offset) T(std::forward<Args>(args)...));
        }

    public:
        [[nodiscard]] auto GetSize() const -> size_t { return m_size; }

    private:
        size_t m_size{0};
        uint8_t *m_data{nullptr};
        size_t m_offset{0};
    };

}
