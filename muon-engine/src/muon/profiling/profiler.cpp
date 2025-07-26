#include "muon/profiling/profiler.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <utility>

namespace muon::profiling {

auto Profiler::createContext(const Spec &spec) -> void {
    core::expect(!s_tracyContext, "tracy context must not exist");

    vk::CommandBufferAllocateInfo commandBufferAi;
    commandBufferAi.commandPool = spec.context->getGraphicsQueue().getCommandPool();
    commandBufferAi.commandBufferCount = 1;
    commandBufferAi.level = vk::CommandBufferLevel::ePrimary;

    auto commandBufferResult = spec.context->getDevice().allocateCommandBuffers(commandBufferAi);
    core::expect(commandBufferResult, "failed to allocate command buffer for profiler");
    auto commandBuffer = std::move((*commandBufferResult)[0]);

    s_tracyContext = TracyVkContext(
        *spec.context->getPhysicalDevice(), *spec.context->getDevice(), *spec.context->getGraphicsQueue().get(), *commandBuffer
    );
    core::debug("created profiler context");
}

auto Profiler::destroyContext() -> void {
    core::expect(s_tracyContext, "tracy context must exist");
    TracyVkDestroy(s_tracyContext);
    core::debug("destroyed profiler context");
}

auto Profiler::collect(vk::raii::CommandBuffer &commandBuffer) -> void { TracyVkCollect(s_tracyContext, *commandBuffer); }

auto Profiler::getContext() -> const tracy::VkCtx * {
    core::expect(s_tracyContext, "tracy context must exist");
    return s_tracyContext;
}

} // namespace muon::profiling
