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

auto ArenaPage::CanFit(size_t size) const -> bool { return (m_offset + size) < m_size; }

ArenaAllocator::ArenaAllocator(size_t size) : m_pageSize{size} { m_pages.push_back(std::make_shared<ArenaPage>(m_pageSize)); }

auto ArenaAllocator::Allocate(size_t size) -> uint8_t * {
    size = Alignment(size, 8);

    auto currentPage = m_pages[m_currentPageIndex];
    if (currentPage == nullptr || !currentPage->CanFit(size)) {
        m_currentPageIndex = m_pages.size();
        m_pages.push_back(std::make_shared<ArenaPage>(m_pageSize));
    }

    return currentPage->Allocate(size);
}

auto ArenaAllocator::Reset() -> void {
    m_pages.clear();
    m_currentPageIndex = 0;
}

} // namespace muon
