#pragma once

#include "muon/graphics/queue_info.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    class QueueAllocator {
    public:
        QueueAllocator(const QueueInfo &queueInfo, const QueueRequestInfo &requestInfo);
        ~QueueAllocator() = default;

        [[nodiscard]] std::vector<VkDeviceQueueCreateInfo> GenerateCreateInfos();

    public:
        [[nodiscard]] uint32_t GetQueueFamilyIndex(const QueueType &queueType) const;
        [[nodiscard]] uint32_t GetNextQueueIndex(const QueueType &queueType);

    private:
        std::unordered_map<QueueType, QueueType> m_aliases{};
        std::unordered_map<QueueType, uint32_t> m_familyIndices{};
        std::unordered_map<QueueType, uint32_t> m_nextQueueIndex{};
        std::unordered_map<QueueType, uint32_t> m_maxQueues{};
    };

}
