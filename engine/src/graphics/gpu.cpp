#include "muon/graphics/gpu.hpp"

#include <string_view>
#include <unordered_set>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    bool GpuSuitability::IsSuitable() const {
        bool core = m_minimumApiSupport && m_discreteGpu && m_minimumPushConstantSize && m_requiredExtensions;
        bool graphics = m_adequateQueues && m_bindlessSupport && m_adequatePresentSupport && m_extraShaders;

        return core && graphics;
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
            suitability.m_minimumApiSupport = true;
        }

        if (deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            suitability.m_discreteGpu = true;
        }

        if (deviceProperties.properties.limits.maxPushConstantsSize >= 128) {
            suitability.m_minimumPushConstantSize = true;
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
            suitability.m_requiredExtensions = true;
        }

        uint32_t queueFamilyPropertyCount{0};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

        const uint32_t minimumQueueCount{3};
        const VkQueueFlags requiredQueueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

        bool minimumRequiredQueues;
        VkQueueFlags totalQueueFlags{};
        for (const auto &properties : queueFamilyProperties) {
            if (properties.queueCount >= 3) {
                minimumRequiredQueues = true;
            }

            totalQueueFlags |= properties.queueFlags;
        }

        if (minimumRequiredQueues && (totalQueueFlags & requiredQueueFlags)) {
            suitability.m_adequateQueues = true;
        }

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &indexingFeatures;
        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

        if (suitability.m_requiredExtensions) {
            suitability.m_bindlessSupport = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;
        }

        if (suitability.m_requiredExtensions) {
            uint32_t surfaceFormatCount{0};
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);

            uint32_t presentModeCount{0};
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &presentModeCount, nullptr);

            suitability.m_adequatePresentSupport = (surfaceFormatCount > 0) && (presentModeCount > 0);
        }

        if (deviceFeatures.features.geometryShader && deviceFeatures.features.tessellationShader) {
            suitability.m_extraShaders = true;
        }

        return suitability;
    }

}
