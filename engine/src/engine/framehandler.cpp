#include "muon/engine/framehandler.hpp"

#include "muon/engine/swapchain.hpp"
#include <print>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    FrameHandler::FrameHandler(Window &window, Device &device) : window(window), device(device) {
        recreateSwapchain();
        createCommandBuffers();
    }

    FrameHandler::~FrameHandler() {
        freeCommandBuffers();
    }

    vk::CommandBuffer FrameHandler::beginFrame() {
        auto result = swapchain->acquireNextImage(&currentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return nullptr;
        }

        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            throw std::runtime_error("failed to acquire next swapchain image");
        }

        frameInProgress = true;

        const auto commandBuffer = getCurrentCommandBuffer();

        vk::CommandBufferBeginInfo beginInfo;

        result = commandBuffer.begin(&beginInfo);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        return commandBuffer;
    }

    void FrameHandler::endFrame() {
        const auto commandBuffer = getCurrentCommandBuffer();
        commandBuffer.end();

        auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.wasResized()) {
            window.resetResized();
            recreateSwapchain();
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swapchain image");
        }

        frameInProgress = false;
        currentFrameIndex = (currentFrameIndex + 1) % constants::maxFramesInFlight;
    }

    void FrameHandler::beginSwapchainRenderPass(vk::CommandBuffer commandBuffer) {
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

    void FrameHandler::endSwapchainRenderPass(vk::CommandBuffer commandBuffer) {
        commandBuffer.endRenderPass();
    }

    void FrameHandler::copyImageToSwapchain(vk::Image image) {
        auto swapchainImage = swapchain->getImage(currentImageIndex);

        const auto commandBuffer = getCurrentCommandBuffer();

        {
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;

            barrier.srcAccessMask = vk::AccessFlagBits{};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

            barrier.image = swapchainImage;

            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eBottomOfPipe,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlagBits{},
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier
            );
        }

        vk::ImageCopy imageCopy{};
        imageCopy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageCopy.srcSubresource.mipLevel = 0;
        imageCopy.srcSubresource.baseArrayLayer = 0;
        imageCopy.srcSubresource.layerCount = 1;
        imageCopy.srcOffset = vk::Offset3D{0, 0, 0};

        imageCopy.dstSubresource = imageCopy.srcSubresource;
        imageCopy.dstOffset = vk::Offset3D{0, 0, 0};

        imageCopy.extent.width = swapchain->getExtent().width;
        imageCopy.extent.height = swapchain->getExtent().height;
        imageCopy.extent.depth = 1;

        commandBuffer.copyImage(
            image,
            vk::ImageLayout::eTransferSrcOptimal,
            swapchainImage,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &imageCopy
        );

        {
            vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;

            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;

            barrier.image = swapchainImage;

            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eBottomOfPipe,
                vk::DependencyFlagBits{},
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier
            );
        }
    }

    vk::RenderPass FrameHandler::getSwapchainRenderPass() const {
        return swapchain->getRenderPass();
    }

    vk::CommandBuffer FrameHandler::getCurrentCommandBuffer() const {
        return commandBuffers[currentFrameIndex];
    }

    void FrameHandler::setClearColor(vk::ClearColorValue newValue) {
        clearColorValue = newValue;
    }

    void FrameHandler::setClearDepthStencil(vk::ClearDepthStencilValue newValue) {
        clearDepthStencilValue = newValue;
    }

    int32_t FrameHandler::getFrameIndex() const {
        return currentFrameIndex;
    }

    bool FrameHandler::isFrameInProgress() const {
        return frameInProgress;
    }

    float FrameHandler::getAspectRatio() const {
        return swapchain->getExtentAspectRatio();
    }

    void FrameHandler::createCommandBuffers() {
        commandBuffers.resize(constants::maxFramesInFlight);

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        auto result = device.getDevice().allocateCommandBuffers(&allocInfo, commandBuffers.data());
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    void FrameHandler::freeCommandBuffers() {
        device.getDevice().freeCommandBuffers(device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void FrameHandler::recreateSwapchain() {
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
            std::shared_ptr oldSwapChain = std::move(swapchain);
            swapchain = std::make_unique<Swapchain>(device, extent, oldSwapChain);
            if (!swapchain->compareSwapFormats(*oldSwapChain)) {
                std::println("swapchain does not match swap formats");
            }
        }
    }

}
