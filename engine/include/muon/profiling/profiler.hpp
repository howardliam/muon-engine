#pragma once

#include "muon/graphics/device_context.hpp"
#include <vulkan/vulkan_core.h>
#include <tracy/TracyVulkan.hpp>

namespace muon::profiling {

    class Profiler {
    public:
        struct Spec {
            const graphics::DeviceContext *deviceContext{nullptr};
        };

    public:
        static void CreateContext(const Spec &spec);
        static void DestroyContext();

        static void Collect(VkCommandBuffer cmd);
        static const tracy::VkCtx *GetContext();

    private:
        static inline tracy::VkCtx *s_tracyContext{nullptr};
    };

}
