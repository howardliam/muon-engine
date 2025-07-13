#pragma once

#include "muon/core/no_copy.hpp"

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

namespace muon {

class ArenaPage {
public:
    ArenaPage(size_t size);

    [[nodiscard]] auto Allocate(size_t size) -> uint8_t *;

public:
    [[nodiscard]] auto CanFit(size_t size) const -> bool;

private:
    size_t m_size;
    uint8_t *m_data{nullptr};
    size_t m_offset{0};
};

template <typename T>
class ArenaPtr : NoCopy {
public:
    ArenaPtr(T *ptr, std::shared_ptr<ArenaPage> page) : m_ptr{ptr}, m_page{page} {};
    ~ArenaPtr() { m_ptr->~T(); }

    ArenaPtr(ArenaPtr &&other) noexcept : m_ptr{other.m_ptr}, m_page{std::move(other.m_page)} { other.m_ptr = nullptr; };
    auto operator=(ArenaPtr &&other) noexcept -> ArenaPtr & {
        if (this != &other) {
            m_page = std::move(other.m_page);
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }
        return *this;
    }

public:
    [[nodiscard]] auto IsValid() const -> bool { return !m_page.expired(); }
    [[nodiscard]] auto Get() const -> T * { return m_ptr; }
    [[nodiscard]] auto operator->() -> T * { return m_ptr; }
    [[nodiscard]] auto operator*() -> T & { return *m_ptr; }

private:
    T *m_ptr{nullptr};
    std::weak_ptr<ArenaPage> m_page{};
};

class ArenaAllocator {
public:
    ArenaAllocator(size_t size);

    template <typename T, typename... Args>
    [[nodiscard]] auto Create(Args &&...args) -> ArenaPtr<T> {
        uint8_t *memory = Allocate(sizeof(T));
        T *ptr = new (memory) T(std::forward<Args>(args)...);
        return ArenaPtr<T>(ptr, m_pages[m_currentPageIndex]);
    }

    auto Reset() -> void;

private:
    [[nodiscard]] auto Allocate(size_t size) -> uint8_t *;

private:
    const size_t m_pageSize;
    std::vector<std::shared_ptr<ArenaPage>> m_pages{};
    size_t m_currentPageIndex{0};
};

} // namespace muon
