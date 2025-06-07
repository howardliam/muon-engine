#pragma once

#include <bitset>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    struct GpuSuitability {
        std::bitset<4> coreRequirements;

        uint64_t memorySize = 0;

        bool IsSuitable() const;

        static GpuSuitability DetermineSuitability(
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface,
            const std::vector<const char *> &deviceExtensions
        );
    };

}
