#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/swapchain.hpp"
#include "muon/engine/window.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class Renderer {
    public:
        Renderer(Window &window, Device &device);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        [[nodiscard]] vk::CommandBuffer beginFrame();
        void endFrame();
        void beginSwapchainRenderPass(vk::CommandBuffer commandBuffer);
        void endSwapchainRenderPass(vk::CommandBuffer commandBuffer);

        [[nodiscard]] vk::RenderPass getSwapchainRenderPass() const;
        [[nodiscard]] vk::CommandBuffer getCurrentCommandBuffer() const;
        void setClearColor(vk::ClearColorValue newValue);
        void setClearDepthStencil(vk::ClearDepthStencilValue newValue);
        [[nodiscard]] int32_t getFrameIndex() const;
        [[nodiscard]] bool isFrameInProgress() const;
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
        bool frameInProgress = false;

        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapchain();
    };

}
