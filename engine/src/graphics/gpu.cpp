#include "muon/graphics/gpu.hpp"

#include <string_view>
#include <unordered_set>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    bool GpuSuitability::IsSuitable() const {
        return coreRequirements == 0b1111;
    }

    GpuSuitability GpuSuitability::DetermineSuitability(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        const std::vector<const char *> &deviceExtensions
    ) {
        GpuSuitability suitability;

        VkPhysicalDeviceProperties2 deviceProperties{};
        deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);

        if (deviceProperties.properties.apiVersion >= VK_API_VERSION_1_3) {
            suitability.coreRequirements.set(3);
        }

        if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            suitability.coreRequirements.set(2);
        }

        if (deviceProperties.properties.limits.maxPushConstantsSize >= 128) {
            suitability.coreRequirements.set(1);
        }

        uint32_t availableExtensionCount{0};
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

        std::unordered_set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto &extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        if (requiredExtensions.empty()) {
            suitability.coreRequirements.set(0);
        }

        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
            if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                suitability.memorySize += memoryProperties.memoryHeaps[i].size;
            }
        }

        return suitability;
    }

}
