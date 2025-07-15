#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

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
    QueueInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    ~QueueInfo() = default;

public:
    [[nodiscard]] uint32_t GetTotalQueueCount() const;
    [[nodiscard]] const std::vector<QueueFamilyInfo> &GetFamilyInfo() const;

private:
    uint32_t m_totalQueueCount{0};
    std::vector<QueueFamilyInfo> m_families{};
};

} // namespace muon::graphics
