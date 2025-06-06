#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    class GpuSuitability {
    public:
        GpuSuitability() = default;
        ~GpuSuitability() = default;

        bool IsSuitable() const;

        static GpuSuitability DetermineSuitability(
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface,
            const std::vector<const char *> &deviceExtensions
        );

    private:
        bool m_minimumApiSupport;
        bool m_discreteGpu;
        bool m_minimumPushConstantSize;
        bool m_requiredExtensions;

        bool m_adequateQueues;
        bool m_bindlessSupport;
        bool m_adequatePresentSupport;
        bool m_extraShaders;
    };

}
