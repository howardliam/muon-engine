#pragma once

#include <bitset>
#include <unordered_set>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    struct GpuSpecification {
        VkPhysicalDevice physicalDevice;
        VkSurfaceKHR surface;
        std::unordered_set<const char *> requiredDeviceExtensions;
        std::unordered_set<const char *> optionalDeviceExtensions;
    };

    class Gpu {
    public:
        Gpu(const GpuSpecification& spec);
        ~Gpu() = default;

    public:
        [[nodiscard]] bool IsSuitable() const;
        [[nodiscard]] uint64_t GetMemorySize() const;
        [[nodiscard]] const std::unordered_set<const char *> &GetSupportedExtensions() const;

    private:
        void DetermineSuitability(
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface,
            const std::unordered_set<const char *> &requiredDeviceExtensions,
            const std::unordered_set<const char *> &optionalDeviceExtensions
        );

    private:
        std::bitset<4> m_coreSuitabilities = 0;
        uint64_t m_memorySize = 0;
        std::unordered_set<const char *> m_supportedExtensions{};
    };

}
