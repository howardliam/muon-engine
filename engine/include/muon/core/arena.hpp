#pragma once

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace muon {

class ArenaPage {
public:
    ArenaPage(size_t size);

    [[nodiscard]] auto Allocate(size_t size) -> uint8_t *;

    auto Retain() -> void;
    auto Release() -> void;

public:
    [[nodiscard]] auto CanFit(size_t size) const -> bool;

private:
    size_t m_size;
    uint8_t *m_data{nullptr};
    size_t m_offset{0};
    std::atomic<size_t> m_refs{0};
};

template <typename T>
class ArenaPtr {
public:
    ArenaPtr(T *ptr, ArenaPage *page) : m_ptr{ptr}, m_page(page) { m_page->Retain(); };

    ~ArenaPtr() {
        m_ptr->~T();
        m_page->Release();
    }

public:
    [[nodiscard]] auto Get() const -> T * { return m_ptr; }
    [[nodiscard]] auto operator->() -> T * { return m_ptr; }
    [[nodiscard]] auto operator*() -> T & { return *m_ptr; }

private:
    T *m_ptr{nullptr};
    ArenaPage *m_page{nullptr};
};

class ArenaAllocator {
public:
    ArenaAllocator(size_t size);

    template <typename T, typename... Args>
    [[nodiscard]] auto Create(Args &&...args) -> ArenaPtr<T> {
        uint8_t *memory = Allocate(sizeof(T));
        T *ptr = new (memory) T(std::forward<Args>(args)...);
        return ArenaPtr<T>(ptr, m_currentPage);
    }

private:
    [[nodiscard]] auto Allocate(size_t size) -> uint8_t *;

private:
    const size_t m_pageSize;
    std::vector<ArenaPage *> m_pages{};
    ArenaPage *m_currentPage{nullptr};
};

} // namespace muon
