#include "muon/renderer/swapchain.hpp"

#include "muon/renderer/device.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include <limits>

namespace muon {

    Swapchain::Swapchain(
        Device &device,
        vk::Extent2D windowExtent
    ) : device(device), windowExtent(windowExtent) {
        init();
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{}", windowExtent.width, windowExtent.height);
    }

    Swapchain::Swapchain(
        Device &device,
        vk::Extent2D windowExtent,
        std::shared_ptr<Swapchain> previous
    ) : device(device), windowExtent(windowExtent), oldSwapchain(previous) {
        init();
        oldSwapchain = nullptr;
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{} from old swapchain", windowExtent.width, windowExtent.height);
    }

    Swapchain::~Swapchain() {
        for (uint32_t i = 0; i < constants::maxFramesInFlight; i++) {
            device.device().destroySemaphore(imageAvailableSemaphores[i], nullptr);
            device.device().destroyFence(inFlightFences[i], nullptr);
        }

        for (uint32_t i = 0; i < swapchainImages.size(); i++) {
            device.device().destroySemaphore(renderFinishedSemaphores[i], nullptr);
        }

        for (auto imageView : swapchainImageViews) {
            device.device().destroyImageView(imageView, nullptr);
        }
        swapchainImageViews.clear();

        device.device().destroySwapchainKHR(swapchain, nullptr);

        MU_CORE_DEBUG("destroyed swapchain");
    }

    vk::Result Swapchain::acquireNextImage(uint32_t *imageIndex) {
        auto result = device.device().waitForFences(1, &inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to wait for fences");

        result = device.device().acquireNextImageKHR(
            swapchain,
            std::numeric_limits<uint64_t>::max(),
            imageAvailableSemaphores[currentFrame],
            nullptr,
            imageIndex
        );

        return result;
    }

    vk::Result Swapchain::submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex) {
        if (imagesInFlight[*imageIndex] != nullptr) {
            auto result = device.device().waitForFences(1, &imagesInFlight[*imageIndex], true, std::numeric_limits<uint64_t>::max());
            MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to wait for fences");
        }
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        submitInfo.pWaitDstStageMask = waitStages;

        vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;

        vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[*imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        auto result = device.device().resetFences(1, &inFlightFences[currentFrame]);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to reset fences");

        result = device.graphicsQueue().submit(1, &submitInfo, inFlightFences[currentFrame]);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to submit draw command buffer");

        vk::PresentInfoKHR presentInfo{};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        vk::SwapchainKHR swapchains[] = { swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;

        presentInfo.pImageIndices = imageIndex;

        result = device.presentQueue().presentKHR(&presentInfo);

        currentFrame = (currentFrame + 1) % constants::maxFramesInFlight;

        return result;
    }

    bool Swapchain::compareSwapFormats(const Swapchain &swapchain) const {
        return swapchain.swapchainImageFormat == swapchainImageFormat;
    }

    size_t Swapchain::imageCount() const {
        return swapchainImages.size();
    }

    vk::SwapchainKHR Swapchain::getSwapchain() const {
        return swapchain;
    }

    vk::Image Swapchain::getImage(int32_t index) const {
        return swapchainImages[index];
    }

    vk::ImageView Swapchain::getImageView(int32_t index) const {
        return swapchainImageViews[index];
    }

    vk::Format Swapchain::getSwapchainImageFormat() const {
        return swapchainImageFormat;
    }

    vk::Extent2D Swapchain::getExtent() const {
        return swapchainExtent;
    }

    uint32_t Swapchain::getWidth() const {
        return swapchainExtent.width;
    }

    uint32_t Swapchain::getHeight() const {
            return swapchainExtent.height;
    }

    float Swapchain::getExtentAspectRatio() const {
        return static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height);
    }

    void Swapchain::init() {
        createSwapchain();
        createImageViews();
        createSyncObjects();
    }

    void Swapchain::createSwapchain() {
        auto selectSurfaceFormat = [](const std::vector<vk::SurfaceFormatKHR> &formats) -> vk::SurfaceFormatKHR {
            for (const auto& format : formats) {
                if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    return format;
                }
            }
            return formats[0];
        };

        auto selectPresentMode = [](const std::vector<vk::PresentModeKHR> &presentModes) -> vk::PresentModeKHR {
            // for (const auto& presentMode : presentModes) {
            //     if (presentMode == vk::PresentModeKHR::eMailbox) {
            //         return presentMode;
            //     }
            // }

            return vk::PresentModeKHR::eFifo;
        };

        auto selectExtent = [](const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D windowExtent) -> vk::Extent2D {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                vk::Extent2D actualExtent = {
                    static_cast<uint32_t>(windowExtent.width),
                    static_cast<uint32_t>(windowExtent.height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        };

        auto details = device.swapchainSupportDetails();

        auto surfaceFormat = selectSurfaceFormat(details.formats);
        auto presentMode = selectPresentMode(details.presentModes);
        auto extent = selectExtent(details.capabilities, windowExtent);

        uint32_t imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
            imageCount = details.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.surface = device.surface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

        auto &indices = device.queueFamilyIndices();
        MU_CORE_ASSERT(indices->isComplete(), "queue family indices must be complete");
        uint32_t queueFamilyIndices[] = {indices->graphicsFamily.value(), indices->presentFamily.value()};

        if (indices->graphicsFamily != indices->presentFamily) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = true;

        createInfo.oldSwapchain = oldSwapchain == nullptr ? nullptr : oldSwapchain->swapchain;

        auto result = device.device().createSwapchainKHR(&createInfo, nullptr, &swapchain);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create the swapchain");

        result = device.device().getSwapchainImagesKHR(swapchain, &imageCount, nullptr);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get swapchain images");
        MU_CORE_ASSERT(imageCount > 0, "there must be more than 0 images");

        swapchainImages.resize(imageCount);

        result = device.device().getSwapchainImagesKHR(swapchain, &imageCount, swapchainImages.data());
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get swapchain images");

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;
    }

    void Swapchain::createImageViews() {
        swapchainImageViews.resize(imageCount());

        for (size_t i = 0; i < imageCount(); i++) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.image = swapchainImages[i];
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = swapchainImageFormat;

            createInfo.components.r = vk::ComponentSwizzle::eR;
            createInfo.components.g = vk::ComponentSwizzle::eG;
            createInfo.components.b = vk::ComponentSwizzle::eB;
            createInfo.components.a = vk::ComponentSwizzle::eA;

            createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            auto result = device.device().createImageView(&createInfo, nullptr, &swapchainImageViews[i]);
            MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create a swapchain image view");
        }
    }

    void Swapchain::createSyncObjects() {
        vk::SemaphoreCreateInfo semaphoreInfo{};

        imageAvailableSemaphores.resize(constants::maxFramesInFlight);
        inFlightFences.resize(constants::maxFramesInFlight);

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (uint32_t i = 0; i < constants::maxFramesInFlight; i++) {
            auto semaphoreRes = device.device().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            auto fenceRes = device.device().createFence(&fenceInfo, nullptr, &inFlightFences[i]);

            MU_CORE_ASSERT(semaphoreRes == vk::Result::eSuccess, "failed to create image available semaphores");
            MU_CORE_ASSERT(fenceRes == vk::Result::eSuccess, "failed to create in flight fences");
        }

        renderFinishedSemaphores.resize(swapchainImages.size());

        for (uint32_t i = 0; i < swapchainImages.size(); i++) {
            auto semaphoreRes = device.device().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);

            MU_CORE_ASSERT(semaphoreRes == vk::Result::eSuccess, "failed to create render finished semaphores");
        }

        imagesInFlight.resize(swapchainImages.size(), nullptr);
    }

}
