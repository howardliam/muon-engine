#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    struct QueueSpecification {
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
        [[nodiscard]] VkQueue Get() const;
        [[nodiscard]] VkCommandPool GetCommandPool() const;

    private:
        VkDevice m_device;
        const char *m_name;

        VkQueue m_queue;
        VkCommandPool m_commandPool;
    };

}
