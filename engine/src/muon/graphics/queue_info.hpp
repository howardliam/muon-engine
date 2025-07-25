#pragma once

#include "vulkan/vulkan_raii.hpp"

#include <bitset>
#include <cstdint>
#include <vector>

namespace muon::graphics {

struct QueueFamilyInfo {
    uint32_t index;
    uint32_t queueCount;
    std::bitset<6> capabilities;

    bool operator==(const QueueFamilyInfo &other) const;

    bool isPresentCapable() const;
    bool isGraphicsCapable() const;
    bool isComputeCapable() const;
    bool isTransferCapable() const;
    bool isVideoDecodeCapable() const;
    bool isVideoEncodeCapable() const;

    bool isComputeDedicated() const;
    bool isTransferDedicated() const;
};

class QueueInfo {
public:
    QueueInfo(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface);

public:
    auto getTotalQueueCount() const -> uint32_t;
    auto getFamilyInfo() const -> const std::vector<QueueFamilyInfo> &;

private:
    uint32_t m_totalQueueCount{0};
    std::vector<QueueFamilyInfo> m_families{};
};

} // namespace muon::graphics
