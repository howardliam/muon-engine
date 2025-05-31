#pragma once

#include "muon/utils/nocopy.hpp"
#include <memory>
#include <vector>
#include <chrono>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Window;
    class Device;
    class Swapchain;

    class FrameHandler : NoCopy {
    public:
        FrameHandler(Window &window, Device &device);
        ~FrameHandler();

        [[nodiscard]] vk::CommandBuffer beginFrame();
        void endFrame();

        void beginFrameTiming();
        void updateFrameTiming();

        void copyImageToSwapchain(vk::Image image);
        void prepareToPresent();

        [[nodiscard]] vk::CommandBuffer getCurrentCommandBuffer() const;
        [[nodiscard]] int32_t getFrameIndex() const;
        [[nodiscard]] bool isFrameInProgress() const;
        [[nodiscard]] float getAspectRatio() const;
        [[nodiscard]] float getFrameTime() const;

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

        void createCommandBuffers();
        void freeCommandBuffers();
    };

}
