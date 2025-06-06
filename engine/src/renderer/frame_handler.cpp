#include "muon/renderer/frame_handler.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/debug/profiler.hpp"
#include "muon/core/window.hpp"
#include "muon/renderer/device.hpp"
#include "muon/renderer/swapchain.hpp"
#include <vulkan/vulkan.hpp>

namespace muon {

    FrameHandler::FrameHandler(Window &window, Device &device) : window(window), device(device) {
        recreateSwapchain(window.extent());
        createCommandBuffers();
        MU_CORE_DEBUG("created frame handler");
    }

    FrameHandler::~FrameHandler() {
        freeCommandBuffers();
        MU_CORE_DEBUG("destroyed frame handler");
    }

    vk::CommandBuffer FrameHandler::beginFrame() {
        auto result = swapchain->acquireNextImage(&currentImageIndex);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain(window.extent());
            return nullptr;
        }

        MU_CORE_ASSERT(result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR, "failed to acquire next swapchain image");

        frameInProgress = true;

        const auto commandBuffer = getCurrentCommandBuffer();

        vk::CommandBufferBeginInfo beginInfo{};

        result = commandBuffer.begin(&beginInfo);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to begin recording command buffer");

        return commandBuffer;
    }

    void FrameHandler::endFrame() {
        const auto commandBuffer = getCurrentCommandBuffer();
        commandBuffer.end();

        Profiler::collect(commandBuffer);

        auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        MU_CORE_ASSERT(
            result == vk::Result::eErrorOutOfDateKHR ||
            result == vk::Result::eSuboptimalKHR ||
            result == vk::Result::eSuccess,
            "failed to present swapchain image"
        );


        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
            recreateSwapchain(window.extent());
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

    void FrameHandler::prepareToPresent() {
        auto swapchainImage = swapchain->getImage(currentImageIndex);

        const auto cmd = getCurrentCommandBuffer();

        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.srcStageMask = vk::PipelineStageFlags2{};
        barrier.srcAccessMask = vk::AccessFlags2{};

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
        allocInfo.commandPool = device.commandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        auto result = device.device().allocateCommandBuffers(&allocInfo, commandBuffers.data());
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to allocate command buffers");
    }

    void FrameHandler::freeCommandBuffers() {
        device.device().freeCommandBuffers(device.commandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void FrameHandler::recreateSwapchain(vk::Extent2D windowExtent) {
        device.device().waitIdle();

        if (swapchain == nullptr) {
            swapchain = std::make_unique<Swapchain>(device, windowExtent);
        } else {
            std::shared_ptr oldSwapChain = std::move(swapchain);
            swapchain = std::make_unique<Swapchain>(device, windowExtent, oldSwapChain);

            if (!swapchain->compareSwapFormats(*oldSwapChain)) {
                MU_CORE_DEBUG("new and old swapchain formats do not match");
            }
        }
    }

}
