#include "muon/graphics/queue_info.hpp"

#include "vulkan/vulkan_enums.hpp"

#include <cstddef>

namespace muon::graphics {

bool QueueFamilyInfo::operator==(const QueueFamilyInfo &other) const {
    return index == other.index && queueCount == other.queueCount && capabilities == other.capabilities;
}

bool QueueFamilyInfo::isPresentCapable() const { return capabilities.test(0); }
bool QueueFamilyInfo::isGraphicsCapable() const { return capabilities.test(1); }
bool QueueFamilyInfo::isComputeCapable() const { return capabilities.test(2); }
bool QueueFamilyInfo::isTransferCapable() const { return capabilities.test(3); }
bool QueueFamilyInfo::isVideoDecodeCapable() const { return capabilities.test(4); }
bool QueueFamilyInfo::isVideoEncodeCapable() const { return capabilities.test(5); }

bool QueueFamilyInfo::isComputeDedicated() const { return !isGraphicsCapable() && isComputeCapable(); }
bool QueueFamilyInfo::isTransferDedicated() const { return !isGraphicsCapable() && !isComputeCapable() && isTransferCapable(); }

QueueInfo::QueueInfo(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface) {
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    m_families.resize(queueFamilyProperties.size());

    size_t index = 0;
    for (const auto &properties : queueFamilyProperties) {

        auto support = physicalDevice.getSurfaceSupportKHR(index, surface);

        std::bitset<6> capabilities;

        if (support) {
            capabilities.set(0);
        }
        if (properties.queueFlags & vk::QueueFlagBits::eGraphics) {
            capabilities.set(1);
        }
        if (properties.queueFlags & vk::QueueFlagBits::eCompute) {
            capabilities.set(2);
        }
        if (properties.queueFlags & vk::QueueFlagBits::eTransfer) {
            capabilities.set(3);
        }
        if (properties.queueFlags & vk::QueueFlagBits::eVideoDecodeKHR) {
            capabilities.set(4);
        }
        if (properties.queueFlags & vk::QueueFlagBits::eVideoEncodeKHR) {
            capabilities.set(5);
        }

        m_families.emplace(m_families.begin() + index, index, properties.queueCount, capabilities);
        m_totalQueueCount += properties.queueCount;

        index += 1;
    }
}

auto QueueInfo::getTotalQueueCount() const -> uint32_t { return m_totalQueueCount; }

auto QueueInfo::getFamilyInfo() const -> const std::vector<QueueFamilyInfo> & { return m_families; }

} // namespace muon::graphics
