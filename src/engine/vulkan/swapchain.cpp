#include "swapchain.hpp"

#include <array>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    Swapchain::Swapchain(logging::Logger logger, Device &device, vk::Extent2D windowExtent) : logger(logger), device(device), windowExtent(windowExtent) {
        try {
            init();
        } catch (std::exception &e) {
            logger->error(e.what());
            std::terminate();
        }
    }

    Swapchain::~Swapchain() {
        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
            device.getDevice().destroySemaphore(imageAvailableSemaphores[i], nullptr);
            device.getDevice().destroySemaphore(renderFinishedSemaphores[i], nullptr);
            device.getDevice().destroyFence(inFlightFences[i], nullptr);
        }
        logger->debug("Destroyed synchronisation objects");

        for (auto framebuffer : swapchainFramebuffers) {
            device.getDevice().destroyFramebuffer(framebuffer, nullptr);
        }
        logger->debug("Destroyed framebuffers");
        swapchainFramebuffers.clear();

        device.getDevice().destroyRenderPass(renderPass, nullptr);
        logger->debug("Destroyed render pass");

        for (size_t i = 0; i < depthImages.size(); i++) {
            device.getDevice().destroyImageView(depthImageViews[i], nullptr);
            device.getAllocator().freeMemory(depthImageAllocations[i]);
            device.getDevice().destroyImage(depthImages[i], nullptr);
        }
        logger->debug("Destroyed depth image views");
        logger->debug("Freed depth image memory");
        logger->debug("Destroyed depth images");

        for (auto imageView : swapchainImageViews) {
            device.getDevice().destroyImageView(imageView, nullptr);
        }
        logger->debug("Destroyed image views");
        swapchainImageViews.clear();

        device.getDevice().destroySwapchainKHR(swapchain, nullptr);
        logger->debug("Destroyed swapchain");
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

        SwapchainSupportDetails supportDetails = device.getSwapchainSupport();

        auto surfaceFormat = selectSurfaceFormat(supportDetails.formats);
        auto presentMode = selectPresentMode(supportDetails.presentModes);
        auto extent = selectExtent(supportDetails.capabilities, windowExtent);

        uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
        if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
            imageCount = supportDetails.capabilities.maxImageCount;
        }

        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
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

        createInfo.oldSwapchain = nullptr;

        auto result = device.getDevice().createSwapchainKHR(&createInfo, nullptr, &swapchain);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create the swapchain");
        }

        logger->debug("Created swapchain");

        result = device.getDevice().getSwapchainImagesKHR(swapchain, &imageCount, nullptr);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get swapchain images");
        }

        swapchainImages.resize(imageCount);

        result = device.getDevice().getSwapchainImagesKHR(swapchain, &imageCount, swapchainImages.data());
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get swapchain images");
        }

        swapchainImageFormat = surfaceFormat.format;
        swapchainExtent = extent;
    }

    void Swapchain::createImageViews() {
        swapchainImageViews.resize(imageCount());

        for (size_t i = 0; i < imageCount(); i++) {
            vk::ImageViewCreateInfo createInfo{};
            createInfo.sType = vk::StructureType::eImageViewCreateInfo;
            createInfo.image = swapchainImages[i];
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = swapchainImageFormat;

            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;

            createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            auto result = device.getDevice().createImageView(&createInfo, nullptr, &swapchainImageViews[i]);
            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create an image view");
            }
        }
        logger->debug("Created image views");
    }

    void Swapchain::createDepthResources() {
        auto findDepthFormat = [&]() -> vk::Format {
            return device.findSupportedFormat(
               { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
               vk::ImageTiling::eOptimal,
               vk::FormatFeatureFlagBits::eDepthStencilAttachment
            );
        };

        depthImageFormat = findDepthFormat();

        depthImages.resize(imageCount());
        depthImageViews.resize(imageCount());
        depthImageAllocations.resize(imageCount());

        for (size_t i = 0; i < depthImages.size(); i++) {
            vk::ImageCreateInfo imageInfo{};
            imageInfo.sType = vk::StructureType::eImageCreateInfo;
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

            device.createImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImages[i], depthImageAllocations[i]);

            vk::ImageViewCreateInfo viewInfo{};
            viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
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
                throw std::runtime_error("Failed to create a depth image view");
            }

        }
        logger->debug("Created depth images");
        logger->debug("Allocated depth image memory");
        logger->debug("Created depth image views");
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
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

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

        vk::SubpassDependency subpassDependency{};
        subpassDependency.dstSubpass = 0;
        subpassDependency.dstAccessMask =
            vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        subpassDependency.dstStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.srcAccessMask = vk::AccessFlags(0);
        subpassDependency.srcStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests;

        std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        vk::RenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = vk::StructureType::eRenderPassCreateInfo;
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        auto result = device.getDevice().createRenderPass(&renderPassCreateInfo, nullptr, &renderPass);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to create render pass");
        }

        logger->debug("Created render pass");
    }

    void Swapchain::createFramebuffers() {
        swapchainFramebuffers.resize(imageCount());

        for (size_t i = 0; i < imageCount(); i++) {
            std::array<vk::ImageView, 2> attachments = { swapchainImageViews[i], depthImageViews[i] };

            vk::FramebufferCreateInfo createInfo{};
            createInfo.sType = vk::StructureType::eFramebufferCreateInfo;
            createInfo.renderPass = renderPass;
            createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            createInfo.pAttachments = attachments.data();
            createInfo.width = swapchainExtent.width;
            createInfo.height = swapchainExtent.height;
            createInfo.layers = 1;

            auto result = device.getDevice().createFramebuffer(&createInfo, nullptr, &swapchainFramebuffers[i]);
            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create a framebuffer");
            }
        }

        logger->debug("Created framebuffers");
    }

    void Swapchain::createSyncObjects() {
        imageAvailableSemaphores.resize(maxFramesInFlight);
        renderFinishedSemaphores.resize(maxFramesInFlight);
        inFlightFences.resize(maxFramesInFlight);

        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
            auto resA = device.getDevice().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            auto resB = device.getDevice().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            auto resC = device.getDevice().createFence(&fenceInfo, nullptr, &inFlightFences[i]);

            if (resA != vk::Result::eSuccess || resB != vk::Result::eSuccess || resC != vk::Result::eSuccess) {
                throw std::runtime_error("Failed to create synchronisation objects");
            }
        }

        logger->debug("Created synchronisation objects");
    }
}
