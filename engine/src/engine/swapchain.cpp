#include "muon/engine/swapchain.hpp"

#include <print>

namespace muon::engine {

    Swapchain::Swapchain(Device &device, vk::Extent2D windowExtent) : device(device), windowExtent(windowExtent) {
        init();
    }

    Swapchain::Swapchain(Device &device, vk::Extent2D windowExtent, std::shared_ptr<Swapchain> previous) : device(device), windowExtent(windowExtent), oldSwapchain(previous) {
        init();
        oldSwapchain = nullptr;
    }

    Swapchain::~Swapchain() {
        for (uint32_t i = 0; i < constants::maxFramesInFlight; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i], nullptr);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i], nullptr);
            device.getDevice().destroyFence(inFlightFences[i], nullptr);
        }

        for (auto framebuffer : swapchainFramebuffers) {
            device.getDevice().destroyFramebuffer(framebuffer, nullptr);
        }
        swapchainFramebuffers.clear();

        device.getDevice().destroyRenderPass(renderPass, nullptr);

        for (size_t i = 0; i < depthImages.size(); i++) {
            device.getDevice().destroyImageView(depthImageViews[i], nullptr);
            device.getAllocator().freeMemory(depthImageAllocations[i]);
            device.getDevice().destroyImage(depthImages[i], nullptr);
        }

        for (auto imageView : swapchainImageViews) {
            device.getDevice().destroyImageView(imageView, nullptr);
        }
        swapchainImageViews.clear();

        device.getDevice().destroySwapchainKHR(swapchain, nullptr);
    }

    vk::Result Swapchain::acquireNextImage(uint32_t *imageIndex) {
        auto result = device.getDevice().waitForFences(1, &inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
        if (result != vk::Result::eSuccess) {
            std::println("failed to wait for fences");
        }

        result = device.getDevice().acquireNextImageKHR(
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
            auto result = device.getDevice().waitForFences(1, &imagesInFlight[*imageIndex], true, UINT64_MAX);
            if (result != vk::Result::eSuccess) {
                std::println("failed to wait for fences");
            }
        }
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        vk::SubmitInfo submitInfo;

        vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        auto result = device.getDevice().resetFences(1, &inFlightFences[currentFrame]);
        if (result != vk::Result::eSuccess) {
            std::println("failed to reset fences");
        }
        result = device.getGraphicsQueue().submit(1, &submitInfo, inFlightFences[currentFrame]);
        if (result != vk::Result::eSuccess) {
            std::println("failed to submit draw command buffer");
        }

        vk::PresentInfoKHR presentInfo = {};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        vk::SwapchainKHR swapchains[] = {swapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;

        presentInfo.pImageIndices = imageIndex;

        result = device.getPresentQueue().presentKHR(&presentInfo);

        currentFrame = (currentFrame + 1) % constants::maxFramesInFlight;

        return result;
    }

    bool Swapchain::compareSwapFormats(const Swapchain &swapchain) const {
        return swapchain.depthImageFormat == depthImageFormat && swapchain.swapchainImageFormat == swapchainImageFormat;
    }

    size_t Swapchain::imageCount() const {
        return swapchainImages.size();
    }

    vk::SwapchainKHR Swapchain::getSwapchain() const {
        return swapchain;
    }

    vk::RenderPass Swapchain::getRenderPass() const {
        return renderPass;
    }

    vk::Framebuffer Swapchain::getFramebuffer(int32_t index) const {
        return swapchainFramebuffers[index];
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
        createDepthResources();
        createRenderPass();
        createFramebuffers();
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
            for (const auto& presentMode : presentModes) {
                if (presentMode == vk::PresentModeKHR::eMailbox) {
                    return presentMode;
                }
            }

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

        SwapchainSupportDetails supportDetails = device.getSwapchainSupportDetails();

        auto surfaceFormat = selectSurfaceFormat(supportDetails.formats);
        auto presentMode = selectPresentMode(supportDetails.presentModes);
        auto extent = selectExtent(supportDetails.capabilities, windowExtent);

        uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
        if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
            imageCount = supportDetails.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.surface = device.getSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

        QueueFamilyIndices indices = device.getQueueFamilyIndices();
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = supportDetails.capabilities.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = presentMode;
        createInfo.clipped = true;

        createInfo.oldSwapchain = oldSwapchain == nullptr ? nullptr : oldSwapchain->swapchain;

        auto result = device.getDevice().createSwapchainKHR(&createInfo, nullptr, &swapchain);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create the swapchain");
        }

        result = device.getDevice().getSwapchainImagesKHR(swapchain, &imageCount, nullptr);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to get swapchain images");
        }

        swapchainImages.resize(imageCount);

        result = device.getDevice().getSwapchainImagesKHR(swapchain, &imageCount, swapchainImages.data());
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to get swapchain images");
        }

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

            auto result = device.getDevice().createImageView(&createInfo, nullptr, &swapchainImageViews[i]);
            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("failed to create an image view");
            }
        }
    }

    void Swapchain::createDepthResources() {
        depthImageFormat = device.findSupportedFormat(
            { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
            vk::ImageTiling::eOptimal,
            vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );

        depthImages.resize(imageCount());
        depthImageViews.resize(imageCount());
        depthImageAllocations.resize(imageCount());

        imagesInFlight.resize(imageCount(), nullptr);

        for (size_t i = 0; i < depthImages.size(); i++) {
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.extent.width = swapchainExtent.width;
            imageInfo.extent.height = swapchainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthImageFormat;
            imageInfo.tiling = vk::ImageTiling::eOptimal;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            imageInfo.samples = vk::SampleCountFlagBits::e1;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.flags = vk::ImageCreateFlags(0);

            device.createImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, vma::MemoryUsage::eGpuOnly, depthImages[i], depthImageAllocations[i]);

            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = depthImages[i];
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = depthImageFormat;

            viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            auto result = device.getDevice().createImageView(&viewInfo, nullptr, &depthImageViews[i]);
            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("failed to create a depth image view");
            }
        }
    }

    void Swapchain::createRenderPass() {
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::eTransferDstOptimal;

        vk::AttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentDescription depthAttachment{};
        depthAttachment.format = depthImageFormat;
        depthAttachment.samples = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        vk::SubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = vk::AccessFlagBits{};
        subpassDependency.dstAccessMask =
            vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        subpassDependency.srcStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests;
        subpassDependency.dstStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests;

        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        vk::RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        auto result = device.getDevice().createRenderPass(&renderPassCreateInfo, nullptr, &renderPass);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create render pass");
        }
    }

    void Swapchain::createFramebuffers() {
        swapchainFramebuffers.resize(imageCount());

        for (size_t i = 0; i < imageCount(); i++) {
            std::array<vk::ImageView, 2> attachments = { swapchainImageViews[i], depthImageViews[i] };

            vk::FramebufferCreateInfo createInfo{};
            createInfo.renderPass = renderPass;
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = swapchainExtent.width;
            createInfo.height = swapchainExtent.height;
            createInfo.layers = 1;

            auto result = device.getDevice().createFramebuffer(&createInfo, nullptr, &swapchainFramebuffers[i]);
            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("failed to create a framebuffer");
            }
        }
    }

    void Swapchain::createSyncObjects() {
        imageAvailableSemaphores.resize(constants::maxFramesInFlight);
        renderFinishedSemaphores.resize(constants::maxFramesInFlight);
        inFlightFences.resize(constants::maxFramesInFlight);

        vk::SemaphoreCreateInfo semaphoreInfo{};

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (uint32_t i = 0; i < constants::maxFramesInFlight; i++) {
            auto semaphoreRes1 = device.getDevice().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            auto semaphoreRes2 = device.getDevice().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            auto fenceRes = device.getDevice().createFence(&fenceInfo, nullptr, &inFlightFences[i]);

            if (semaphoreRes1 != vk::Result::eSuccess || semaphoreRes2 != vk::Result::eSuccess || fenceRes != vk::Result::eSuccess) {
                throw std::runtime_error("failed to create synchronisation objects");
            }
        }
    }

}
