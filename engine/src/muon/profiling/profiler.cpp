#include "muon/profiling/profiler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::profiling {

auto Profiler::CreateContext(const Spec &spec) -> void {
    MU_CORE_ASSERT(s_tracyContext == nullptr, "tracy context must not exist");

    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = spec.context->GetGraphicsQueue().GetCommandPool();
    allocateInfo.commandBufferCount = 1;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(spec.context->GetDevice(), &allocateInfo, &cmd);

    s_tracyContext =
        TracyVkContext(spec.context->GetPhysicalDevice(), spec.context->GetDevice(), spec.context->GetGraphicsQueue().Get(), cmd);
    MU_CORE_DEBUG("created profiler context");
}

auto Profiler::DestroyContext() -> void {
    MU_CORE_ASSERT(s_tracyContext != nullptr, "tracy context must exist");
    TracyVkDestroy(s_tracyContext);
    MU_CORE_DEBUG("destroyed profiler context");
}

auto Profiler::Collect(VkCommandBuffer cmd) -> void { TracyVkCollect(s_tracyContext, cmd); }

auto Profiler::GetContext() -> const tracy::VkCtx * {
    MU_CORE_ASSERT(s_tracyContext != nullptr, "tracy context must exist");
    return s_tracyContext;
}

} // namespace muon::profiling
