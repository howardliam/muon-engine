#pragma once

#include "muon/graphics/swapchain.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    class FrameManager : NoCopy, NoMove {
    public:
        FrameManager();
        ~FrameManager();

        [[nodiscard]] VkCommandBuffer BeginFrame();
        void EndFrame();

    public:

    private:
        void CreateSwapchain();
        void CreateCommandBuffers();

    private:
        std::unique_ptr<Swapchain> m_swapchain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex = 0;
        int32_t m_currentFrameIndex = 0;
        bool m_frameInProgress = false;
    };

}
