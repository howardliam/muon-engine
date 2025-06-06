#include "muon/debug/profiler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace muon {

    void Profiler::Collect(VkCommandBuffer cmd) {
        TracyVkCollect(s_tracyContext, cmd);
    }

    tracy::VkCtx *Profiler::Context() {
        MU_CORE_ASSERT(s_tracyContext, "tracy context must exist");
        return s_tracyContext;
    }

    void Profiler::CreateContext(VkPhysicalDevice pd, VkDevice d, VkQueue gq, VkCommandBuffer cmd) {
        s_tracyContext = TracyVkContext(pd, d, gq, cmd);
        MU_CORE_DEBUG("created profiler context");
    }

    void Profiler::DestroyContext() {
        MU_CORE_ASSERT(s_tracyContext, "tracy context must exist");
        TracyVkDestroy(s_tracyContext);
        MU_CORE_DEBUG("destroyed profiler context");
    }

}
