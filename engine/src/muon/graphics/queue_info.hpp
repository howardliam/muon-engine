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

    bool IsPresentCapable() const;
    bool IsGraphicsCapable() const;
    bool IsComputeCapable() const;
    bool IsTransferCapable() const;
    bool IsVideoDecodeCapable() const;
    bool IsVideoEncodeCapable() const;

    bool IsComputeDedicated() const;
    bool IsTransferDedicated() const;
};

class QueueInfo {
public:
    QueueInfo(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface);
    ~QueueInfo() = default;

public:
    uint32_t GetTotalQueueCount() const;
    const std::vector<QueueFamilyInfo> &GetFamilyInfo() const;

private:
    uint32_t m_totalQueueCount{0};
    std::vector<QueueFamilyInfo> m_families{};
};

} // namespace muon::graphics
