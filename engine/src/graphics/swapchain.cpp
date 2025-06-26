#include "muon/graphics/swapchain.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/base.hpp"
#include "muon/core/log.hpp"
#include <algorithm>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    Swapchain::Swapchain(const SwapchainSpecification &spec) : m_deviceContext(*spec.deviceContext) {
        CreateSwapchain(spec.windowExtent, spec.oldSwapchain);
        CreateImageViews();
        CreateSyncObjects();

        if (spec.oldSwapchain) {
            MU_CORE_DEBUG("created swapchain with dimensions: {}x{} from old swapchain", m_swapchainExtent.width, m_swapchainExtent.height);
        } else {
            MU_CORE_DEBUG("created swapchain with dimensions: {}x{}", m_swapchainExtent.width, m_swapchainExtent.height);
        }
    }

    Swapchain::~Swapchain() {
        for (auto &semaphore : m_imageAvailableSemaphores) {
            vkDestroySemaphore(m_deviceContext.GetDevice(), semaphore, nullptr);
        }

        for (auto &fence : m_inFlightFences) {
            vkDestroyFence(m_deviceContext.GetDevice(), fence, nullptr);
        }

        for (auto &semaphore : m_renderFinishedSemaphores) {
            vkDestroySemaphore(m_deviceContext.GetDevice(), semaphore, nullptr);
        }

        for (auto imageView : m_swapchainImageViews) {
            vkDestroyImageView(m_deviceContext.GetDevice(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(m_deviceContext.GetDevice(), m_swapchain, nullptr);

        MU_CORE_DEBUG("destroyed swapchain");
    }

    VkResult Swapchain::AcquireNextImage(uint32_t *imageIndex) {
        auto result = vkWaitForFences(m_deviceContext.GetDevice(), 1, &m_inFlightFences[m_currentFrame], true, std::numeric_limits<uint64_t>::max());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to wait for fences");

        result = vkAcquireNextImageKHR(
            m_deviceContext.GetDevice(),
            m_swapchain,
            std::numeric_limits<uint64_t>::max(),
            m_imageAvailableSemaphores[m_currentFrame],
            nullptr,
            imageIndex
        );

        return result;
    }

    VkResult Swapchain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
        if (m_imagesInFlight[*imageIndex] != nullptr) {
            auto result = vkWaitForFences(m_deviceContext.GetDevice(), 1, &m_imagesInFlight[*imageIndex], true, std::numeric_limits<uint64_t>::max());
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

        auto result = vkResetFences(m_deviceContext.GetDevice(), 1, &m_inFlightFences[m_currentFrame]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to reset fences");

        result = vkQueueSubmit(m_deviceContext.GetGraphicsQueue().Get(), 1, &submitInfo, m_inFlightFences[m_currentFrame]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to submit draw command buffer");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_swapchain;

        presentInfo.pImageIndices = imageIndex;

        result = vkQueuePresentKHR(m_deviceContext.GetGraphicsQueue().Get(), &presentInfo);

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

    void Swapchain::CreateSwapchain(const VkExtent2D windowExtent, VkSwapchainKHR oldSwapchain) {
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

        auto selectSurfaceFormat = [](const std::vector<VkSurfaceFormat2KHR> &formats) -> VkSurfaceFormatKHR {
            // order matters here, prefer HDR and A2R10G10B10 before falling back to R8G8B8A8 sRGB
            const std::vector<std::pair<VkFormat, VkColorSpaceKHR>> preferredSurfaceFormats = {
                { VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_COLOR_SPACE_HDR10_ST2084_EXT },
                { VK_FORMAT_A2R10G10B10_UNORM_PACK32, VK_COLOR_SPACE_BT2020_LINEAR_EXT },
                { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            };

            uint32_t matchIndex = preferredSurfaceFormats.size(); // start at end of preferences vector
            const VkSurfaceFormatKHR *surfaceFormatMatch = nullptr;

            for (const auto &format : formats) {
                for (uint32_t i = 0; i < preferredSurfaceFormats.size(); i++) {
                    const auto &surfaceFormat = preferredSurfaceFormats[i];
                    if (format.surfaceFormat.format == surfaceFormat.first && format.surfaceFormat.colorSpace == surfaceFormat.second) {
                        if (i < matchIndex) {
                            matchIndex = i;
                            surfaceFormatMatch = &format.surfaceFormat;
                        }
                        break;
                    }
                }
            }

            MU_CORE_ASSERT(surfaceFormatMatch, "there must be a surface format match");

            if (surfaceFormatMatch->format == VK_FORMAT_A2R10G10B10_UNORM_PACK32) {
                MU_CORE_TRACE("format selected: A2R10G10B10");
            } else {
                MU_CORE_TRACE("format selected: R8G8B8A8");
            }

            if (surfaceFormatMatch->colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT) {
                MU_CORE_TRACE("colour space selected: HDR10 ST2084");
            } else if (surfaceFormatMatch->colorSpace == VK_COLOR_SPACE_BT2020_LINEAR_EXT) {
                MU_CORE_TRACE("colour space selected: BT2020 Linear");
            } else {
                MU_CORE_TRACE("colour space selected: sRGB Nonlinear");
            }

            return *surfaceFormatMatch;
        };

        auto selectPresentMode = [](const std::vector<VkPresentModeKHR> &presentModes) -> VkPresentModeKHR {
            auto it = std::ranges::find_if(presentModes, [](const VkPresentModeKHR &mode) { return mode == VK_PRESENT_MODE_IMMEDIATE_KHR; });
            if (it != presentModes.end()) { return *it; }

            return VK_PRESENT_MODE_FIFO_KHR;
        };

        VkResult result;

        VkSurfaceCapabilitiesKHR capabilities{};
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &capabilities);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface capabilities");

        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{};
        surfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
        surfaceInfo.surface = m_deviceContext.GetSurface();

        uint32_t surfaceFormatCount = 0;
        result = vkGetPhysicalDeviceSurfaceFormats2KHR(m_deviceContext.GetPhysicalDevice(), &surfaceInfo, &surfaceFormatCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface format count");
        std::vector<VkSurfaceFormat2KHR> surfaceFormats(surfaceFormatCount);
        for (auto &format : surfaceFormats) {
            format.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        }
        result = vkGetPhysicalDeviceSurfaceFormats2KHR(m_deviceContext.GetPhysicalDevice(), &surfaceInfo, &surfaceFormatCount, surfaceFormats.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface formats");

        uint32_t presentModeCount = 0;
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &presentModeCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present modes");
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &presentModeCount, presentModes.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present mode count");

        auto extent = selectExtent(capabilities, windowExtent);
        auto surfaceFormat = selectSurfaceFormat(surfaceFormats);
        auto presentMode = selectPresentMode(presentModes);

        uint32_t minImageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
            minImageCount = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_deviceContext.GetSurface();
        createInfo.minImageCount = minImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = true;

        createInfo.oldSwapchain = oldSwapchain;

        result = vkCreateSwapchainKHR(m_deviceContext.GetDevice(), &createInfo, nullptr, &m_swapchain);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create the swapchain");

        result = vkGetSwapchainImagesKHR(m_deviceContext.GetDevice(), m_swapchain, &m_imageCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get swapchain image count");
        m_swapchainImages.resize(m_imageCount);
        result = vkGetSwapchainImagesKHR(m_deviceContext.GetDevice(), m_swapchain, &m_imageCount, m_swapchainImages.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get swapchain images");

        m_swapchainImageFormat = surfaceFormat.format;
        m_swapchainExtent = extent;
    }

    void Swapchain::CreateImageViews() {
        m_swapchainImageViews.resize(m_imageCount);

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

            auto result = vkCreateImageView(m_deviceContext.GetDevice(), &createInfo, nullptr, &m_swapchainImageViews[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create a swapchain image view");
        }
    }

    void Swapchain::CreateSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_imageAvailableSemaphores.resize(constants::maxFramesInFlight);
        m_inFlightFences.resize(constants::maxFramesInFlight);
        for (uint32_t i = 0; i < constants::maxFramesInFlight; i++) {
            VkResult result;

            result = vkCreateSemaphore(m_deviceContext.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create image available semaphores");

            result = vkCreateFence(m_deviceContext.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create in flight fences");
        }

        m_renderFinishedSemaphores.resize(m_imageCount);
        for (uint32_t i = 0; i < m_imageCount; i++) {
            auto result = vkCreateSemaphore(m_deviceContext.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create render finished semaphores");
        }

        m_imagesInFlight.resize(m_imageCount, nullptr);
    }

}
