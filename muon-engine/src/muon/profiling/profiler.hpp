#pragma once

#include "muon/graphics/context.hpp"
#include "tracy/TracyVulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace muon::profiling {

class Profiler {
public:
    struct Spec {
        const graphics::Context *context{nullptr};
    };

public:
    static auto createContext(const Spec &spec) -> void;
    static auto destroyContext() -> void;

    static auto collect(vk::raii::CommandBuffer &commandBuffer) -> void;
    static auto getContext() -> const tracy::VkCtx *;

private:
    static inline tracy::VkCtx *s_tracyContext{nullptr};
};

} // namespace muon::profiling
