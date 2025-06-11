#pragma once

#include "muon/graphics/queue_info.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    struct QueueSpecification {
        QueueType type;
        VkDevice device;
        uint32_t queueFamilyIndex;
        uint32_t queueIndex;
        const char *name;
    };

    class Queue : NoCopy, NoMove {
    public:
        Queue(const QueueSpecification &spec);
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
        const char *m_name;

        VkQueue m_queue;
        VkCommandPool m_commandPool;
    };

}
