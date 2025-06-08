#include "muon/graphics/swapchain.hpp"

#include "muon/core/application.hpp"
#include "muon/core/log.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    Swapchain::Swapchain() {
        Init();
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{}", m_swapchainExtent.width, m_swapchainExtent.height);
    }

    Swapchain::Swapchain(std::shared_ptr<Swapchain> previous) : m_oldSwapchain(previous) {
        Init();
        m_oldSwapchain = nullptr;
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{} from old swapchain", m_swapchainExtent.width, m_swapchainExtent.height);
    }

    Swapchain::~Swapchain() {
        auto &context = Application::Get().GetGraphicsContext();

        for (auto &semaphore : m_imageAvailableSemaphores) {
            vkDestroySemaphore(context.GetDevice(), semaphore, nullptr);
        }

        for (auto &fence : m_inFlightFences) {
            vkDestroyFence(context.GetDevice(), fence, nullptr);
        }

        for (auto &semaphore : m_renderFinishedSemaphores) {
            vkDestroySemaphore(context.GetDevice(), semaphore, nullptr);
        }

        for (auto imageView : m_swapchainImageViews) {
            vkDestroyImageView(context.GetDevice(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(context.GetDevice(), m_swapchain, nullptr);

        MU_CORE_DEBUG("destroyed swapchain");
    }

    VkResult Swapchain::AcquireNextImage(uint32_t *imageIndex) {
        auto &context = Application::Get().GetGraphicsContext();

        auto result = vkWaitForFences(context.GetDevice(), 1, &m_inFlightFences[m_currentFrame], true, std::numeric_limits<uint64_t>::max());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to wait for fences");

        result = vkAcquireNextImageKHR(
            context.GetDevice(),
            m_swapchain,
            std::numeric_limits<uint64_t>::max(),
            m_imageAvailableSemaphores[m_currentFrame],
            nullptr,
            imageIndex
        );

        return result;
    }

    VkResult Swapchain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
        auto &context = Application::Get().GetGraphicsContext();

        if (m_imagesInFlight[*imageIndex] != nullptr) {
            auto result = vkWaitForFences(context.GetDevice(), 1, &m_imagesInFlight[*imageIndex], true, std::numeric_limits<uint64_t>::max());
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to wait for fences");
        }
        m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.pWaitDstStageMask = waitStages;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[*imageIndex] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        auto result = vkResetFences(context.GetDevice(), 1, &m_inFlightFences[m_currentFrame]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to reset fences");

        result = vkQueueSubmit(context.GetGraphicsQueue().Get(), 1, &submitInfo, m_inFlightFences[m_currentFrame]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to submit draw command buffer");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;

        presentInfo.pImageIndices = imageIndex;

        result = vkQueuePresentKHR(context.GetPresentQueue().Get(), &presentInfo);

        m_currentFrame = (m_currentFrame + 1) % constants::maxFramesInFlight;

        return result;
    }

    bool Swapchain::CompareSwapFormats(const Swapchain &other) const {
        return m_swapchainImageFormat == other.m_swapchainImageFormat;
    }

    size_t Swapchain::GetImageCount() const {
        return m_imageCount;
    }

    VkSwapchainKHR Swapchain::GetSwapchain() const {
        return m_swapchain;
    }

    VkFormat Swapchain::GetSwapchainImageFormat() const {
        return m_swapchainImageFormat;
    }

    VkImage Swapchain::GetImage(int32_t index) const {
        return m_swapchainImages[index];
    }

    VkImageView Swapchain::GetImageView(int32_t index) const {
        return m_swapchainImageViews[index];
    }

    VkExtent2D Swapchain::GetExtent() const {
        return m_swapchainExtent;
    }

    uint32_t Swapchain::GetWidth() const {
        return m_swapchainExtent.width;
    }

    uint32_t Swapchain::GetHeight() const {
        return m_swapchainExtent.height;
    }

    float Swapchain::GetExtentAspectRatio() const {
        return static_cast<float>(m_swapchainExtent.width) / m_swapchainExtent.height;
    }

    void Swapchain::Init() {
        CreateSwapchain();
        CreateImageViews();
        CreateSyncObjects();
    }

    void Swapchain::CreateSwapchain() {
        auto selectExtent = [](const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D windowExtent) -> VkExtent2D {
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {
                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(windowExtent.width),
                    static_cast<uint32_t>(windowExtent.height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        };

        auto selectSurfaceFormat = [](const std::vector<VkSurfaceFormatKHR> &formats) -> VkSurfaceFormatKHR {
            for (const auto& format : formats) {
                if (format.format ==  VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return format;
                }
            }
            return formats[0];
        };

        auto selectPresentMode = [](const std::vector<VkPresentModeKHR> &presentModes) -> VkPresentModeKHR {
            return VK_PRESENT_MODE_FIFO_KHR;
        };

        auto &context = Application::Get().GetGraphicsContext();
        auto &window = Application::Get().GetWindow();
        VkResult result;

        VkSurfaceCapabilitiesKHR capabilities{};
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.GetPhysicalDevice(), context.GetSurface(), &capabilities);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface capabilities");

        uint32_t surfaceFormatCount = 0;
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(context.GetPhysicalDevice(), context.GetSurface(), &surfaceFormatCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface formats");
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(context.GetPhysicalDevice(), context.GetSurface(), &surfaceFormatCount, surfaceFormats.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface formats");

        uint32_t presentModeCount = 0;
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(context.GetPhysicalDevice(), context.GetSurface(), &presentModeCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present modes");
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(context.GetPhysicalDevice(), context.GetSurface(), &presentModeCount, presentModes.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present modes");

        auto extent = selectExtent(capabilities, window.GetExtent());
        auto surfaceFormat = selectSurfaceFormat(surfaceFormats);
        auto presentMode = selectPresentMode(presentModes);

        uint32_t minImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
            minImageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = context.GetSurface();
        createInfo.minImageCount = minImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        const auto &indices = context.GetQueueIndices();
        const uint32_t queueFamilyIndices[] = { indices.graphics, indices.present };
        if (indices.graphics != indices.present) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = true;

        createInfo.oldSwapchain = m_oldSwapchain == nullptr ? nullptr : m_oldSwapchain->m_swapchain;

        result = vkCreateSwapchainKHR(context.GetDevice(), &createInfo, nullptr, &m_swapchain);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create the swapchain");

        result = vkGetSwapchainImagesKHR(context.GetDevice(), m_swapchain, &m_imageCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get swapchain images");
        m_swapchainImages.resize(m_imageCount);
        result = vkGetSwapchainImagesKHR(context.GetDevice(), m_swapchain, &m_imageCount, m_swapchainImages.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get swapchain images");

        m_swapchainImageFormat = surfaceFormat.format;
        m_swapchainExtent = extent;
    }

    void Swapchain::CreateImageViews() {
        m_swapchainImageViews.resize(m_imageCount);

        auto &context = Application::Get().GetGraphicsContext();
        for (size_t i = 0; i < m_imageCount; i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapchainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(context.GetDevice(), &createInfo, nullptr, &m_swapchainImageViews[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create a swapchain image view");
        }
    }

    void Swapchain::CreateSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto &context = Application::Get().GetGraphicsContext();

        m_imageAvailableSemaphores.resize(constants::maxFramesInFlight);
        m_inFlightFences.resize(constants::maxFramesInFlight);
        for (uint32_t i = 0; i < constants::maxFramesInFlight; i++) {
            VkResult result;

            result = vkCreateSemaphore(context.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create image available semaphores");

            result = vkCreateFence(context.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create in flight fences");
        }

        m_renderFinishedSemaphores.resize(m_imageCount);
        for (uint32_t i = 0; i < m_imageCount; i++) {
            auto result = vkCreateSemaphore(context.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create render finished semaphores");
        }

        m_imagesInFlight.resize(m_imageCount, nullptr);
    }

}
