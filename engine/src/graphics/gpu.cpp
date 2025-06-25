#include "muon/graphics/gpu.hpp"

#include <string_view>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    Gpu::Gpu(const GpuSpecification& spec) {
        DetermineSuitability(
            spec.physicalDevice,
            spec.surface,
            spec.requiredDeviceExtensions,
            spec.optionalDeviceExtensions
        );
    }

    bool Gpu::IsSuitable() const {
        return m_coreSuitabilities == 0b1111;
    }

    uint64_t Gpu::GetMemorySize() const {
        return m_memorySize;
    }

    const std::unordered_set<const char *> &Gpu::GetSupportedExtensions() const {
        return m_supportedExtensions;
    }

    void Gpu::DetermineSuitability(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        const std::unordered_set<const char *> &requiredDeviceExtensions,
        const std::unordered_set<const char *> &optionalDeviceExtensions
    ) {
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        if (deviceProperties.apiVersion >= VK_API_VERSION_1_3) {
            m_coreSuitabilities.set(3);
        }

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            m_coreSuitabilities.set(2);
        }

        if (deviceProperties.limits.maxPushConstantsSize >= 128) {
            m_coreSuitabilities.set(1);
        }

        uint32_t availableExtensionCount{0};
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

        std::unordered_set<std::string_view> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
        std::unordered_set<std::string_view> optionalExtensions(optionalDeviceExtensions.begin(), optionalDeviceExtensions.end());
        for (const auto &extension : availableExtensions) {
            if (requiredExtensions.contains(extension.extensionName)) {
                m_supportedExtensions.insert(extension.extensionName);
                requiredExtensions.erase(extension.extensionName);
            }

            if (optionalExtensions.contains(extension.extensionName)) {
                m_supportedExtensions.insert(extension.extensionName);
                requiredExtensions.erase(extension.extensionName);
            }
        }

        if (requiredExtensions.empty()) {
            m_coreSuitabilities.set(0);
        }

        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
            if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                m_memorySize += memoryProperties.memoryHeaps[i].size;
            }
        }
    }

}
