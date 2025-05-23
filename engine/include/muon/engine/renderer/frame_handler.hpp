#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include <memory>
#include <vector>
#include <chrono>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class Window;
    class Device;
    class Swapchain;

    /**
     * @brief   handles the swapchain, recording commands, and presenting frames.
     */
    class FrameHandler : NoCopy {
    public:
        FrameHandler(Window &window, Device &device);
        ~FrameHandler();

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

        void beginFrameTiming();

        void updateFrameTiming();

        void copyImageToSwapchain(vk::Image image);

        /**
         * @brief   gets the current command buffer.
         *
         * @return  command buffer handle.
         */
        [[nodiscard]] vk::CommandBuffer getCurrentCommandBuffer() const;

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

        [[nodiscard]] float getFrameTime() const;

        /**
         * @brief   recreates the swapchain, used for when window size changes, etc.
         */
        void recreateSwapchain(vk::Extent2D windowExtent);

    private:
        Window &window;
        Device &device;

        std::unique_ptr<Swapchain> swapchain;
        std::vector<vk::CommandBuffer> commandBuffers;

        uint32_t currentImageIndex{0};
        int32_t currentFrameIndex{0};
        bool frameInProgress{false};

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime;
        float frameTime{0.0};

        /**
         * @brief   creates command buffers for each image in the swapchain.
         */
        void createCommandBuffers();

        /**
         * @brief   frees the allocated command buffers.
         */
        void freeCommandBuffers();


    };

}
