#pragma once

#include "vulkan/vulkan_raii.hpp"

#include <bitset>

namespace muon::graphics {

class Gpu {
public:
    struct Spec {
        const vk::raii::PhysicalDevice *physicalDevice{nullptr};
    };

public:
    Gpu(const Spec &spec);
    ~Gpu() = default;

public:
    auto isSuitable() const -> bool;
    auto getMemorySize() const -> uint64_t;

    auto getPhysicalDevice() const -> const vk::raii::PhysicalDevice &;

private:
    auto determineSuitability() -> void;

private:
    const vk::raii::PhysicalDevice *m_physicalDevice;

    std::bitset<4> m_coreSuitabilities{0};
    uint64_t m_memorySize{0};
};

} // namespace muon::graphics
