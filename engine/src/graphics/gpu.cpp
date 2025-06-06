#include "muon/graphics/gpu.hpp"

#include <bitset>

namespace muon::gfx {

    bool GpuSuitability::operator==(const GpuSuitability &other) const {
        return m_suitability == other.m_suitability;
    }

    bool GpuSuitability::GetFlag(const SuitabilityFlagBits &flag) const {
        return m_suitability & static_cast<uint64_t>(flag);
    }

    void GpuSuitability::SetFlag(const SuitabilityFlagBits &flag) {
        m_suitability |= static_cast<uint64_t>(flag);
    }

    std::string GpuSuitability::ToString() const {
        return std::bitset<64>(m_suitability).to_string();
    }

}
