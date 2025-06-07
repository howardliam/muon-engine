#pragma once

#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    enum class QueueType {
        Graphics,
        Present,
        Compute,
        Transfer,
    };

    class Queue {
    public:
        Queue(QueueType type, VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex);
        ~Queue();

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
