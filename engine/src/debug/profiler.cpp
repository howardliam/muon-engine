#include "muon/debug/profiler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include <vulkan/vulkan_core.h>

namespace muon {

    void Profiler::CreateContext(const ProfilerSpecification &spec) {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = spec.deviceContext->GetGraphicsQueue().GetCommandPool();
        allocateInfo.commandBufferCount = 1;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(spec.deviceContext->GetDevice(), &allocateInfo, &cmd);

        s_tracyContext = TracyVkContext(
            spec.deviceContext->GetPhysicalDevice(),
            spec.deviceContext->GetDevice(),
            spec.deviceContext->GetGraphicsQueue().Get(),
            cmd
        );
        MU_CORE_DEBUG("created profiler context");
    }

    void Profiler::DestroyContext() {
        MU_CORE_ASSERT(s_tracyContext, "tracy context must exist");
        TracyVkDestroy(s_tracyContext);
        MU_CORE_DEBUG("destroyed profiler context");
    }

    void Profiler::Collect(VkCommandBuffer cmd) {
        TracyVkCollect(s_tracyContext, cmd);
    }

    tracy::VkCtx *Profiler::GetContext() {
        MU_CORE_ASSERT(s_tracyContext, "tracy context must exist");
        return s_tracyContext;
    }

}
