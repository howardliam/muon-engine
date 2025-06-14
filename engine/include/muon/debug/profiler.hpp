#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/queue_context.hpp"
#include <vulkan/vulkan_core.h>
#include <tracy/TracyVulkan.hpp>

namespace muon::gfx {
    class DeviceContext;
}

namespace muon {

    class Profiler {
    public:
        static void CreateContext(const gfx::DeviceContext &deviceContext, const gfx::QueueContext &queueContext);
        static void DestroyContext();


        static void Collect(VkCommandBuffer cmd);
        static tracy::VkCtx *GetContext();

    private:
        static inline tracy::VkCtx *s_tracyContext{nullptr};
    };

}
