#include "muon/engine/debug/profiler.hpp"

namespace muon {

    tracy::VkCtx *Profiler::s_tracyContext{nullptr};

    void Profiler::collect(vk::CommandBuffer cmd) {
        TracyVkCollect(s_tracyContext, cmd);
    }

    tracy::VkCtx *Profiler::context() {
        return s_tracyContext;
    }

    void Profiler::createContext(vk::PhysicalDevice pd, vk::Device d, vk::Queue gq, vk::CommandBuffer cmd) {
        s_tracyContext = TracyVkContext(pd, d, gq, cmd);
    }

    void Profiler::destroyContext() {
        TracyVkDestroy(s_tracyContext);
    }

}
