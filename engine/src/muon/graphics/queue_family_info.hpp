#pragma once

#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <vector>

namespace muon::graphics {

class QueueFamilyInfo {
public:
    QueueFamilyInfo(vk::QueueFlags capabilities, bool presentSupport, uint32_t index, uint32_t queueCount);

public:
    auto isCapable(vk::QueueFlagBits capability) const -> bool;
    auto supportsPresent() const -> bool;

    auto getIndex() const -> uint32_t;
    auto getQueueCount() const -> uint32_t;

    auto operator==(const QueueFamilyInfo &other) const -> bool;

private:
    vk::QueueFlags m_capabilities;
    bool m_presentSupport;

    uint32_t m_index;
    uint32_t m_queueCount;
};

auto generateQueueFamilyInfo(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface)
    -> std::vector<QueueFamilyInfo>;

} // namespace muon::graphics
