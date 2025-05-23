#include "muon/engine/renderer/frame_handler.hpp"

#include "muon/engine/renderer/device.hpp"
#include "muon/engine/renderer/swapchain.hpp"
#include "muon/engine/renderer/window.hpp"
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include "muon/engine/log/logger.hpp"

namespace mu {

    FrameHandler::FrameHandler(Window &window, Device &device) : window(window), device(device) {
        recreateSwapchain(window.getExtent());
        createCommandBuffers();
        log::globalLogger->debug("created frame handler");
    }

    FrameHandler::~FrameHandler() {
        freeCommandBuffers();
        log::globalLogger->debug("destroyed frame handler");
    }

    vk::CommandBuffer FrameHandler::beginFrame() {
        auto result = swapchain->acquireNextImage(&currentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain(window.getExtent());
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

        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            recreateSwapchain(window.getExtent());
        } else if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to present swapchain image");
        }

        frameInProgress = false;
        currentFrameIndex = (currentFrameIndex + 1) % constants::maxFramesInFlight;
    }

    void FrameHandler::beginFrameTiming() {
        currentTime = std::chrono::high_resolution_clock::now();
        frameTime = 0.0;
    }

    void FrameHandler::updateFrameTiming() {
        auto newTime = std::chrono::high_resolution_clock::now();
        frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
    }

    void FrameHandler::copyImageToSwapchain(vk::Image image) {
        auto swapchainImage = swapchain->getImage(currentImageIndex);

        const auto cmd = getCurrentCommandBuffer();

        {
            vk::ImageMemoryBarrier2 barrier{};
            barrier.oldLayout = vk::ImageLayout::eUndefined;
            barrier.srcStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
            barrier.srcAccessMask = vk::AccessFlagBits2{};
            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;

            barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
            barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

            barrier.image = swapchainImage;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vk::DependencyInfo dependencyInfo{};
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &barrier;

            cmd.pipelineBarrier2(dependencyInfo);
        }

        vk::ImageCopy imageCopy{};
        imageCopy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageCopy.srcSubresource.mipLevel = 0;
        imageCopy.srcSubresource.baseArrayLayer = 0;
        imageCopy.srcSubresource.layerCount = 1;
        imageCopy.srcOffset = vk::Offset3D{0, 0, 0};

        imageCopy.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageCopy.dstSubresource.mipLevel = 0;
        imageCopy.dstSubresource.baseArrayLayer = 0;
        imageCopy.dstSubresource.layerCount = 1;
        imageCopy.dstOffset = vk::Offset3D{0, 0, 0};

        imageCopy.extent.width = swapchain->getExtent().width;
        imageCopy.extent.height = swapchain->getExtent().height;
        imageCopy.extent.depth = 1;

        cmd.copyImage(
            image,
            vk::ImageLayout::eTransferSrcOptimal,
            swapchainImage,
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &imageCopy
        );

        {
            vk::ImageMemoryBarrier2 barrier{};
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
            barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;

            barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
            barrier.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;
            barrier.dstAccessMask = vk::AccessFlagBits2::eMemoryRead;

            barrier.image = swapchainImage;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vk::DependencyInfo dependencyInfo{};
            dependencyInfo.imageMemoryBarrierCount = 1;
            dependencyInfo.pImageMemoryBarriers = &barrier;

            cmd.pipelineBarrier2(dependencyInfo);
        }
    }

    vk::CommandBuffer FrameHandler::getCurrentCommandBuffer() const {
        return commandBuffers[currentFrameIndex];
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

    float FrameHandler::getFrameTime() const {
        return frameTime;
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

    void FrameHandler::recreateSwapchain(vk::Extent2D windowExtent) {
        device.getDevice().waitIdle();

        if (swapchain == nullptr) {
            swapchain = std::make_unique<Swapchain>(device, windowExtent);
        } else {
            std::shared_ptr oldSwapChain = std::move(swapchain);
            swapchain = std::make_unique<Swapchain>(device, windowExtent, oldSwapChain);

            if (!swapchain->compareSwapFormats(*oldSwapChain)) {
                log::globalLogger->debug("new and old swapchain formats do not match");
            }
        }
    }

}
