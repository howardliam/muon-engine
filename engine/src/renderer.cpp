#include "muon/engine/renderer.hpp"
#include "muon/engine/swapchain.hpp"

#include <print>

namespace muon::engine {

    Renderer::Renderer(Window &window, Device &device) : window(window), device(device) {
        recreateSwapchain();
        createCommandBuffers();
    }

    Renderer::~Renderer() {
        freeCommandBuffers();
    }

    vk::RenderPass Renderer::getSwapchainRenderPass() const {
        return swapchain->getRenderPass();
    }

    vk::CommandBuffer Renderer::getCurrentCommandBuffer() const {
        return commandBuffers[currentFrameIndex];
    }

    void Renderer::setClearColor(vk::ClearColorValue newValue) {
        clearColorValue = newValue;
    }

    void Renderer::setClearDepthStencil(vk::ClearDepthStencilValue newValue) {
        clearDepthStencilValue = newValue;
    }

    int32_t Renderer::getFrameIndex() const {
        return currentFrameIndex;
    }

    bool Renderer::isFrameInProgress() const {
        return frameInProgress;
    }

    float Renderer::getAspectRatio() const {
        return swapchain->getExtentAspectRatio();
    }


    vk::CommandBuffer Renderer::beginFrame() {
        auto result = swapchain->acquireNextImage(&currentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return nullptr;
        }

        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            std::println("failed to acquire next swapchain image");
        }

        frameInProgress = true;

        const auto commandBuffer = getCurrentCommandBuffer();

        vk::CommandBufferBeginInfo beginInfo;

        result = commandBuffer.begin(&beginInfo);
        if (result != vk::Result::eSuccess) {
            std::println("failed to begin recording command buffer");
        }

        return commandBuffer;
    }

    void Renderer::endFrame() {
        const auto commandBuffer = getCurrentCommandBuffer();
        commandBuffer.end();

        auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.wasResized()) {
            window.resetResized();
            recreateSwapchain();
        } else if (result != vk::Result::eSuccess) {
            std::println("failed to present swapchain image");
        }

        frameInProgress = false;
        currentFrameIndex = (currentFrameIndex + 1) % constants::maxFramesInFlight;
    }

    void Renderer::beginSwapchainRenderPass(vk::CommandBuffer commandBuffer) {
        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.renderPass = swapchain->getRenderPass();
        renderPassInfo.framebuffer = swapchain->getFramebuffer(currentImageIndex);

        renderPassInfo.renderArea.setOffset({0, 0});
        renderPassInfo.renderArea.extent = swapchain->getExtent();

        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = clearColorValue;
        clearValues[1].depthStencil = clearDepthStencilValue;

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->getWidth());
        viewport.height = static_cast<float>(swapchain->getHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor{};
        scissor.setOffset({0, 0});
        scissor.extent = swapchain->getExtent();

        commandBuffer.setViewport(0, 1, &viewport);
        commandBuffer.setScissor(0, 1, &scissor);
    }

    void Renderer::endSwapchainRenderPass(vk::CommandBuffer commandBuffer) {
        commandBuffer.endRenderPass();
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(constants::maxFramesInFlight);

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        auto result = device.getDevice().allocateCommandBuffers(&allocInfo, commandBuffers.data());
        if (result != vk::Result::eSuccess) {
            std::println("failed to allocate command buffers");
        }
    }

    void Renderer::freeCommandBuffers() {
        device.getDevice().freeCommandBuffers(device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void Renderer::recreateSwapchain() {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            SDL_Event event;
            SDL_WaitEvent(&event);
        }

        device.getDevice().waitIdle();

        if (swapchain == nullptr) {
            swapchain = std::make_unique<Swapchain>(device, extent);
        } else {
            std::shared_ptr old_swap_chain = std::move(swapchain);
            swapchain = std::make_unique<Swapchain>(device, extent, old_swap_chain);
            if (!swapchain->compareSwapFormats(*old_swap_chain)) {
                std::println("swapchain does not match swap formats");
            }
        }
    }

}
