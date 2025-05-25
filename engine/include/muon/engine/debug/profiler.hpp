#pragma once

#include <vulkan/vulkan.hpp>
#include <tracy/TracyVulkan.hpp>

namespace muon {

    class Profiler {
    public:
        static void collect(vk::CommandBuffer cmd);
        static tracy::VkCtx *context();

    private:
        static void createContext(vk::PhysicalDevice pd, vk::Device d, vk::Queue gq, vk::CommandBuffer cmd);
        static void destroyContext();

        friend class Device;

    private:
        static tracy::VkCtx *s_tracyContext;
    };

}
