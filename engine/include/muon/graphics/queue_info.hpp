#pragma once

#include <bitset>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    enum class QueueType {
        Graphics,
        Compute,
        Transfer,
        Present,
    };

    struct QueueFamilyInfo {
        uint32_t familyIndex;
        uint32_t maxQueues;
        std::bitset<4> capabilities;

        bool operator==(const QueueFamilyInfo &other) const;

        bool IsPresentCapable() const;
        bool IsGraphicsCapable() const;
        bool IsComputeCapable() const;
        bool IsTransferCapable() const;

        bool IsComputeDedicated() const;
        bool IsTransferDedicated() const;
    };

    struct QueueRequestInfo {
        uint32_t graphicsCount = 0;
        uint32_t computeCount = 0;
        uint32_t transferCount = 0;
        uint32_t presentCount = 0;
    };

    class QueueInfo {
    public:
        QueueInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
        ~QueueInfo() = default;

    public:
        [[nodiscard]] const std::vector<QueueFamilyInfo> &GetFamilyInfo() const;

    private:
        std::vector<QueueFamilyInfo> m_families;
    };

}
