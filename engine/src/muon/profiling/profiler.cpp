#include "muon/profiling/profiler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <utility>

namespace muon::profiling {

auto Profiler::CreateContext(const Spec &spec) -> void {
    MU_CORE_ASSERT(s_tracyContext == nullptr, "tracy context must not exist");

    vk::CommandBufferAllocateInfo commandBufferAi;
    commandBufferAi.commandPool = spec.context->GetGraphicsQueue().GetCommandPool();
    commandBufferAi.commandBufferCount = 1;
    commandBufferAi.level = vk::CommandBufferLevel::ePrimary;

    auto commandBufferResult = spec.context->GetDevice().allocateCommandBuffers(commandBufferAi);
    MU_CORE_ASSERT(commandBufferResult, "failed to allocate command buffer for profiler");
    auto commandBuffer = std::move((*commandBufferResult)[0]);

    s_tracyContext = TracyVkContext(
        *spec.context->GetPhysicalDevice(), *spec.context->GetDevice(), *spec.context->GetGraphicsQueue().Get(), *commandBuffer
    );
    MU_CORE_DEBUG("created profiler context");
}

auto Profiler::DestroyContext() -> void {
    MU_CORE_ASSERT(s_tracyContext != nullptr, "tracy context must exist");
    TracyVkDestroy(s_tracyContext);
    MU_CORE_DEBUG("destroyed profiler context");
}

auto Profiler::Collect(vk::raii::CommandBuffer &commandBuffer) -> void { TracyVkCollect(s_tracyContext, *commandBuffer); }

auto Profiler::GetContext() -> const tracy::VkCtx * {
    MU_CORE_ASSERT(s_tracyContext != nullptr, "tracy context must exist");
    return s_tracyContext;
}

} // namespace muon::profiling
