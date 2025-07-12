#pragma once

#include "muon/graphics/device_context.hpp"

#include <tracy/TracyVulkan.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::profiling {

class Profiler {
public:
    struct Spec {
        const graphics::DeviceContext *device{nullptr};
    };

public:
    static auto CreateContext(const Spec &spec) -> void;
    static auto DestroyContext() -> void;

    static auto Collect(VkCommandBuffer cmd) -> void;
    static auto GetContext() -> const tracy::VkCtx *;

private:
    static inline tracy::VkCtx *s_tracyContext{nullptr};
};

} // namespace muon::profiling
