#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/swapchain.hpp"
#include "muon/engine/window.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    /**
     * @brief   handles the swapchain, recording commands, and presenting frames.
     */
    class FrameHandler {
    public:
        FrameHandler(Window &window, Device &device);
        ~FrameHandler();

        FrameHandler(const FrameHandler &) = delete;
        FrameHandler &operator=(const FrameHandler &) = delete;

        /**
         * @brief   provides a command buffer to begin recording commands.
         *
         * @return  command buffer to record commands into for the current frame.
         */
        [[nodiscard]] vk::CommandBuffer beginFrame();

        /**
         * @brief   ends the current frame being recorded.
         */
        void endFrame();

        /**
         * @brief   begins the swapchain render pass.
         *
         * @param   commandBuffer   the command buffer currently being recorded into for the frame.
         */
        void beginSwapchainRenderPass(vk::CommandBuffer commandBuffer);

        /**
         * @brief   ends the swapchain render pass.
         *
         * @param   commandBuffer   the command buffer currently being recorded into for the frame.
         */
        void endSwapchainRenderPass(vk::CommandBuffer commandBuffer);

        void copyImageToSwapchain(vk::Image image);

        /**
         * @brief   gets the render pass from the swapchain.
         *
         * @return  rende pass handle.
         */
        [[nodiscard]] vk::RenderPass getSwapchainRenderPass() const;

        /**
         * @brief   gets the current command buffer.
         *
         * @return  command buffer handle.
         */
        [[nodiscard]] vk::CommandBuffer getCurrentCommandBuffer() const;

        /**
         * @brief   sets the swapchain clear color.
         *
         * @param   new color to set the background to.
         */
        void setClearColor(vk::ClearColorValue newValue);

        /**
         * @brief   sets the swapchain clear depth stencil.
         *
         * @param   new depth stencil value to set.
         */
        void setClearDepthStencil(vk::ClearDepthStencilValue newValue);

        /**
         * @brief   gets the current frame index.
         *
         * @return  frame index.
         */
        [[nodiscard]] int32_t getFrameIndex() const;

        /**
         * @brief   gets the state of frame.
         *
         * @return  whether the frame is in progress.
         */
        [[nodiscard]] bool isFrameInProgress() const;

        /**
         * @brief   gets the aspect ratio of the swapchain image.
         *
         * @return  aspect ratio.
         */
        [[nodiscard]] float getAspectRatio() const;

    private:
        Window &window;
        Device &device;

        std::unique_ptr<Swapchain> swapchain;
        std::vector<vk::CommandBuffer> commandBuffers;

        vk::ClearColorValue clearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
        vk::ClearDepthStencilValue clearDepthStencilValue{1.0f, 0};

        uint32_t currentImageIndex{0};
        int32_t currentFrameIndex{0};
        bool frameInProgress{false};

        /**
         * @brief   creates command buffers for each image in the swapchain.
         */
        void createCommandBuffers();

        /**
         * @brief   frees the allocated command buffers.
         */
        void freeCommandBuffers();

        /**
         * @brief   recreates the swapchain, used for when window size changes, etc.
         */
        void recreateSwapchain();
    };

}
