#include "muon/core/arena.hpp"

#include "muon/utils/alignment.hpp"

#include <cstdlib>

namespace muon {

ArenaPage::ArenaPage(size_t size) : m_size{size} { m_data = static_cast<uint8_t *>(std::malloc(size)); }

auto ArenaPage::Allocate(size_t size) -> uint8_t * {
    uint8_t *memory = m_data + m_offset;
    m_offset += size;
    return memory;
}

auto ArenaPage::Retain() -> void { m_refs.fetch_add(1, std::memory_order::relaxed); }

auto ArenaPage::Release() -> void {
    if (m_refs.fetch_sub(1, std::memory_order::acq_rel) == 1) {
        delete this;
    }
}

auto ArenaPage::CanFit(size_t size) const -> bool { return (m_offset + size) < m_size; }

ArenaAllocator::ArenaAllocator(size_t size) : m_pageSize{size} {}

auto ArenaAllocator::Allocate(size_t size) -> uint8_t * {
    size = Alignment(size, 8);
    if (m_currentPage == nullptr || !m_currentPage->CanFit(size)) {
        m_currentPage = new ArenaPage(m_pageSize);
        m_pages.push_back(m_currentPage);
    }

    return m_currentPage->Allocate(size);
}

} // namespace muon
