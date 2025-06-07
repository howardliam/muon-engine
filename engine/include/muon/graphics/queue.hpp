#pragma once

#include <vector>
#include <unordered_set>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    enum class QueueType {
        Graphics,
        Present,
        Compute,
        Transfer,
    };

    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;

        uint32_t present; // will usually be the same as graphics on desktop discrete GPUs.
        QueueType presentQueueType;

        std::unordered_set<uint32_t> UniqueQueues() const;
        std::vector<VkDeviceQueueCreateInfo> GenerateQueueCreateInfos();

        static QueueFamilyIndices DetermineIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    };

    class Queue {
    public:
        Queue(QueueType type, VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex);
        ~Queue();

    public:
    [[nodiscard]] VkCommandBuffer BeginCommands();
    void EndCommands(VkCommandBuffer cmd);

    public:
        [[nodiscard]] QueueType GetType() const;

        [[nodiscard]] VkQueue Get() const;
        [[nodiscard]] VkCommandPool GetCommandPool() const;

    private:
        QueueType m_type;

        VkQueue m_queue;
        VkCommandPool m_commandPool;
    };

}
