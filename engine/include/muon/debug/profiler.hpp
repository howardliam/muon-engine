#pragma once

#include <vulkan/vulkan_core.h>
#include <tracy/TracyVulkan.hpp>

namespace muon::gfx {
    class Context;
}

namespace muon {

    class Profiler {
    public:
        static void Collect(VkCommandBuffer cmd);
        static tracy::VkCtx *Context();

    private:
        static void CreateContext(VkPhysicalDevice pd, VkDevice d, VkQueue gq, VkCommandBuffer cmd);
        static void DestroyContext();

        friend class gfx::Context;

    private:
        static inline tracy::VkCtx *s_tracyContext{nullptr};
    };

}
