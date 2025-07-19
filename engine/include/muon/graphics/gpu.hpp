#pragma once

#include <bitset>
#include <string>
#include <unordered_set>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Gpu {
public:
    struct Spec {
        VkPhysicalDevice physicalDevice{nullptr};
        VkSurfaceKHR surface{nullptr};
        std::unordered_set<const char *> requiredDeviceExtensions{};
        std::unordered_set<const char *> optionalDeviceExtensions{};
    };

public:
    Gpu(const Spec &spec);
    ~Gpu() = default;

public:
    bool IsSuitable() const;
    uint64_t GetMemorySize() const;
    const std::unordered_set<std::string> &GetSupportedExtensions() const;

private:
    void DetermineSuitability(
        VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::unordered_set<const char *> &requiredDeviceExtensions,
        const std::unordered_set<const char *> &optionalDeviceExtensions
    );

private:
    std::bitset<4> m_coreSuitabilities{0};
    uint64_t m_memorySize{0};
    std::unordered_set<std::string> m_supportedExtensions{};
};

} // namespace muon::graphics
