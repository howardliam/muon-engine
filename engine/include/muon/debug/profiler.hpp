#pragma once

#include "muon/graphics/device_context.hpp"
#include <vulkan/vulkan_core.h>
#include <tracy/TracyVulkan.hpp>

namespace muon::debug {

    struct ProfilerSpecification {
        const graphics::DeviceContext *deviceContext = nullptr;
    };

    class Profiler {
    public:
        static void CreateContext(const ProfilerSpecification &spec);
        static void DestroyContext();

        static void Collect(VkCommandBuffer cmd);
        static tracy::VkCtx *GetContext();

    private:
        static inline tracy::VkCtx *s_tracyContext = nullptr;
    };

}
