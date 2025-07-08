#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <limits>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    class Queue : NoCopy, NoMove {
    public:
        struct Spec {
            VkDevice device{nullptr};
            uint32_t queueFamilyIndex{std::numeric_limits<uint32_t>().max()};
            uint32_t queueIndex{std::numeric_limits<uint32_t>().max()};
            const char *name{nullptr};
        };

    public:
        Queue(const Spec &spec);
        ~Queue();

    public:
        [[nodiscard]] VkCommandBuffer BeginCommands();
        void EndCommands(VkCommandBuffer cmd);

    public:
        [[nodiscard]] VkQueue Get() const;
        [[nodiscard]] VkCommandPool GetCommandPool() const;

    private:
        VkDevice m_device{nullptr};
        const char *m_name{nullptr};

        VkQueue m_queue{nullptr};
        VkCommandPool m_commandPool{nullptr};
    };

}
