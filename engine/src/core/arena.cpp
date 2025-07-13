#include "muon/core/arena.hpp"

#include "muon/utils/alignment.hpp"

#include <cstdlib>

namespace muon {

ArenaPage::ArenaPage(size_t size) : size{size} { data = static_cast<uint8_t *>(std::malloc(size)); }

auto ArenaPage::Retain() -> void { refs.fetch_add(1, std::memory_order::relaxed); }

auto ArenaPage::Release() -> void {
    if (refs.fetch_sub(1, std::memory_order::acq_rel) == 1) {
        delete this;
    }
}

ArenaAllocator::ArenaAllocator(size_t size) : m_pageSize{size} {}

auto ArenaAllocator::Allocate(size_t size) -> uint8_t * {
    size = Alignment(size, 8);
    if (m_currentPage == nullptr || (m_currentPage->offset + size) > m_currentPage->size) {
        m_currentPage = new ArenaPage(m_pageSize);
        m_pages.push_back(m_currentPage);
    }

    uint8_t *memory = m_currentPage->data + m_currentPage->offset;
    m_currentPage->offset += size;

    return memory;
}

} // namespace muon
