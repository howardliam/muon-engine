#include "muon/graphics/queue_info.hpp"

#include "muon/core/assert.hpp"

namespace muon::graphics {

bool QueueFamilyInfo::operator==(const QueueFamilyInfo &other) const {
    return index == other.index && queueCount == other.queueCount && capabilities == other.capabilities;
}

bool QueueFamilyInfo::IsPresentCapable() const { return capabilities.test(0); }
bool QueueFamilyInfo::IsGraphicsCapable() const { return capabilities.test(1); }
bool QueueFamilyInfo::IsComputeCapable() const { return capabilities.test(2); }
bool QueueFamilyInfo::IsTransferCapable() const { return capabilities.test(3); }
bool QueueFamilyInfo::IsVideoDecodeCapable() const { return capabilities.test(4); }
bool QueueFamilyInfo::IsVideoEncodeCapable() const { return capabilities.test(5); }

bool QueueFamilyInfo::IsComputeDedicated() const { return !IsGraphicsCapable() && IsComputeCapable(); }
bool QueueFamilyInfo::IsTransferDedicated() const { return !IsGraphicsCapable() && !IsComputeCapable() && IsTransferCapable(); }

QueueInfo::QueueInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t queueFamilyPropertyCount{0};
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

    m_families.resize(queueFamilyPropertyCount);

    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++) {
        const auto &properties = queueFamilyProperties[i];

        VkBool32 presentSupport;
        auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface support");

        std::bitset<6> capabilities;

        if (presentSupport) {
            capabilities.set(0);
        }
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            capabilities.set(1);
        }
        if (properties.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            capabilities.set(2);
        }
        if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            capabilities.set(3);
        }
        if (properties.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
            capabilities.set(4);
        }
        if (properties.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            capabilities.set(5);
        }

        m_families.emplace(m_families.begin() + i, i, properties.queueCount, capabilities);
        m_totalQueueCount += properties.queueCount;
    }
}

uint32_t QueueInfo::GetTotalQueueCount() const { return m_totalQueueCount; }

const std::vector<QueueFamilyInfo> &QueueInfo::GetFamilyInfo() const { return m_families; }

} // namespace muon::graphics
