#pragma once

#include "vulkan/vulkan_raii.hpp"

#include <bitset>
#include <string>
#include <unordered_set>

namespace muon::graphics {

class Gpu {
public:
    struct Spec {
        const vk::raii::PhysicalDevice *physicalDevice{nullptr};
        const vk::raii::SurfaceKHR *surface{nullptr};
    };

public:
    Gpu(const Spec &spec);
    ~Gpu() = default;

public:
    bool IsSuitable() const;
    uint64_t GetMemorySize() const;
    const std::unordered_set<std::string> &GetSupportedExtensions() const;

private:
    void DetermineSuitability(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface);

private:
    std::bitset<4> m_coreSuitabilities{0};
    uint64_t m_memorySize{0};
    std::unordered_set<std::string> m_supportedExtensions{};
};

} // namespace muon::graphics
