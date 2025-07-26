#include "muon/graphics/queue_family_info.hpp"

namespace muon::graphics {

QueueFamilyInfo::QueueFamilyInfo(vk::QueueFlags capabilities, bool presentSupport, uint32_t index, uint32_t queueCount)
    : m_capabilities{capabilities}, m_presentSupport{presentSupport}, m_index{index}, m_queueCount{queueCount} {}

auto QueueFamilyInfo::isCapable(vk::QueueFlagBits capability) const -> bool {
    return static_cast<bool>(m_capabilities & capability);
}
auto QueueFamilyInfo::supportsPresent() const -> bool { return m_presentSupport; }

auto QueueFamilyInfo::getIndex() const -> uint32_t { return m_index; }
auto QueueFamilyInfo::getQueueCount() const -> uint32_t { return m_queueCount; }

auto QueueFamilyInfo::operator==(const QueueFamilyInfo &other) const -> bool {
    return m_index == other.m_index && m_queueCount == other.m_queueCount && m_capabilities == other.m_capabilities &&
           m_presentSupport == other.m_presentSupport;
}

auto generateQueueFamilyInfo(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface)
    -> std::vector<QueueFamilyInfo> {
    std::vector<QueueFamilyInfo> families;

    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    size_t index = 0;
    for (const auto &properties : queueFamilyProperties) {
        auto support = physicalDevice.getSurfaceSupportKHR(index, surface);
        families.emplace_back(properties.queueFlags, support, index, properties.queueCount);
        index += 1;
    }

    return families;
}

} // namespace muon::graphics
