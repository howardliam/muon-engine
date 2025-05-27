#include "muon/debug/profiler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace muon {

    void Profiler::collect(vk::CommandBuffer cmd) {
        TracyVkCollect(s_tracyContext, cmd);
    }

    tracy::VkCtx *Profiler::context() {
        MU_CORE_ASSERT(s_tracyContext, "tracy context must exist");
        return s_tracyContext;
    }

    void Profiler::createContext(vk::PhysicalDevice pd, vk::Device d, vk::Queue gq, vk::CommandBuffer cmd) {
        s_tracyContext = TracyVkContext(pd, d, gq, cmd);
        MU_CORE_DEBUG("created profiler context");
    }

    void Profiler::destroyContext() {
        TracyVkDestroy(s_tracyContext);
        MU_CORE_DEBUG("destroyed profiler context");
    }

}
