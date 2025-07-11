#pragma once

#include "muon/core/log.hpp"
#include "muon/utils/alignment.hpp"
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <utility>

namespace muon {

    struct ArenaPage {
        static constexpr size_t k_size{4096};
        uint8_t *data{static_cast<uint8_t *>(std::malloc(k_size))};
        size_t offset{0};

        std::atomic<size_t> refCount{0};

        auto Retain() -> void {
            refCount.fetch_add(1, std::memory_order::relaxed);
        }

        auto Release() -> void {
            if (refCount.fetch_sub(1, std::memory_order::acq_rel) == 1) {
                delete this;
            }
        }
    };

    template<typename T>
    class ArenaPtr {
    public:
        ArenaPtr(T *ptr, ArenaPage *page) : m_ptr{ptr}, m_page(page) {
            m_page->Retain();
        };

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
        ArenaAllocator() = default;
        ~ArenaAllocator() = default;

        template<typename T, typename ...Args>
        [[nodiscard]] auto Create(Args &&...args) -> ArenaPtr<T> {
            uint8_t *memory = Allocate(sizeof(T));
            T *ptr = new(memory) T(std::forward<Args>(args)...);
            return ArenaPtr<T>(ptr, m_currentPage);
        }

    private:
        [[nodiscard]] auto Allocate(size_t size) -> uint8_t * {
            size = Alignment(size, 8);
            if (m_currentPage == nullptr || (m_currentPage->offset + size) > ArenaPage::k_size) {
                m_currentPage = new ArenaPage();
                m_pages.push_back(m_currentPage);
                MU_CORE_DEBUG("allocated new arena page");
            }

            uint8_t *memory =  m_currentPage->data + m_currentPage->offset;
            m_currentPage->offset += size;

            return memory;
        }

    private:
        std::vector<ArenaPage *> m_pages{};
        ArenaPage *m_currentPage{nullptr};
    };

}
