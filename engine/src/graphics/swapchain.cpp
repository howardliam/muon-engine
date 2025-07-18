#include "muon/graphics/swapchain.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <algorithm>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

constexpr uint64_t k_waitDuration = 30'000'000'000;

Swapchain::Swapchain(const Spec &spec)
    : m_context(*spec.context), m_graphicsQueue(m_context.GetGraphicsQueue()), m_swapchainColorSpace(spec.colorSpace),
      m_swapchainFormat(spec.format) {
    CreateSwapchain(spec.windowExtent, spec.presentMode, spec.oldSwapchain);
    CreateImageViews();
    CreateSyncObjects();

    if (spec.oldSwapchain) {
        MU_CORE_DEBUG(
            "created swapchain with dimensions: {}x{} from old swapchain", m_swapchainExtent.width, m_swapchainExtent.height
        );
    } else {
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{}", m_swapchainExtent.width, m_swapchainExtent.height);
    }
}

Swapchain::~Swapchain() {
    for (auto semaphore : m_imageAvailableSemaphores) {
        vkDestroySemaphore(m_context.GetDevice(), semaphore, nullptr);
    }

    for (auto fence : m_inFlightFences) {
        vkDestroyFence(m_context.GetDevice(), fence, nullptr);
    }

    for (auto semaphore : m_renderFinishedSemaphores) {
        vkDestroySemaphore(m_context.GetDevice(), semaphore, nullptr);
    }

    for (auto imageView : m_swapchainImageViews) {
        vkDestroyImageView(m_context.GetDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_context.GetDevice(), m_swapchain, nullptr);

    MU_CORE_DEBUG("destroyed swapchain");
}

auto Swapchain::AcquireNextImage(uint32_t *imageIndex) -> VkResult {
    auto result = vkWaitForFences(m_context.GetDevice(), 1, &m_inFlightFences[m_currentFrame], true, k_waitDuration);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to wait for fences");

    result = vkAcquireNextImageKHR(
        m_context.GetDevice(), m_swapchain, k_waitDuration, m_imageAvailableSemaphores[m_currentFrame], nullptr, imageIndex
    );

    return result;
}

auto Swapchain::SubmitCommandBuffers(const VkCommandBuffer cmd, uint32_t imageIndex) -> VkResult {
    if (m_imagesInFlight[imageIndex] != nullptr) {
        auto result = vkWaitForFences(m_context.GetDevice(), 1, &m_imagesInFlight[imageIndex], true, k_waitDuration);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to wait for fences");
    }
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

    auto result = vkResetFences(m_context.GetDevice(), 1, &m_inFlightFences[m_currentFrame]);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to reset fences");

    VkSubmitInfo2 submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};

    VkSemaphoreSubmitInfo waitSemaphores{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    waitSemaphores.semaphore = m_imageAvailableSemaphores[m_currentFrame];
    waitSemaphores.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphores;

    VkCommandBufferSubmitInfo commandBufferInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO};
    commandBufferInfo.commandBuffer = cmd;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferInfo;

    VkSemaphoreSubmitInfo signalSemaphores{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
    signalSemaphores.semaphore = m_renderFinishedSemaphores[imageIndex];
    signalSemaphores.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphores;

    result = vkQueueSubmit2(m_graphicsQueue.Get(), 1, &submitInfo, m_inFlightFences[m_currentFrame]);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to submit to queue");

    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphores.semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_graphicsQueue.Get(), &presentInfo);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to present queue");

    m_currentFrame = (m_currentFrame + 1) % k_maxFramesInFlight;

    return result;
}

auto Swapchain::GetImageCount() const -> size_t { return m_imageCount; }

auto Swapchain::Get() const -> VkSwapchainKHR { return m_swapchain; }

auto Swapchain::GetFormat() const -> VkFormat { return m_swapchainFormat; }

auto Swapchain::IsImageHdr() const -> bool {
    switch (m_swapchainColorSpace) {
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: {
            return true;
        }

        default: {
            return false;
        }
    }
}

auto Swapchain::GetImage(int32_t index) const -> VkImage { return m_swapchainImages[index]; }

auto Swapchain::GetImageView(int32_t index) const -> VkImageView { return m_swapchainImageViews[index]; }

auto Swapchain::GetExtent() const -> VkExtent2D { return m_swapchainExtent; }

auto Swapchain::GetWidth() const -> uint32_t { return m_swapchainExtent.width; }

auto Swapchain::GetHeight() const -> uint32_t { return m_swapchainExtent.height; }

auto Swapchain::GetAspectRatio() const -> float { return static_cast<float>(m_swapchainExtent.width) / m_swapchainExtent.height; }

auto Swapchain::CreateSwapchain(VkExtent2D windowExtent, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain) -> void {
    VkSurfaceCapabilitiesKHR capabilities{};
    auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.GetPhysicalDevice(), m_context.GetSurface(), &capabilities);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface capabilities");

    VkExtent2D extent{};
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        extent.width = std::clamp(windowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(windowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    uint32_t minImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) { // prevent too many images
        minImageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    createInfo.surface = m_context.GetSurface();
    createInfo.minImageCount = minImageCount;
    createInfo.imageFormat = m_swapchainFormat;
    createInfo.imageColorSpace = m_swapchainColorSpace;
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

    result = vkCreateSwapchainKHR(m_context.GetDevice(), &createInfo, nullptr, &m_swapchain);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create the swapchain");

    result = vkGetSwapchainImagesKHR(m_context.GetDevice(), m_swapchain, &m_imageCount, nullptr);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get swapchain image count");
    m_swapchainImages.resize(m_imageCount);
    result = vkGetSwapchainImagesKHR(m_context.GetDevice(), m_swapchain, &m_imageCount, m_swapchainImages.data());
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get swapchain images");

    m_swapchainExtent = extent;
}

auto Swapchain::CreateImageViews() -> void {
    m_swapchainImageViews.resize(m_imageCount);

    bool swizzle = false;
    if (m_swapchainFormat == VK_FORMAT_B8G8R8A8_SRGB || m_swapchainFormat == VK_FORMAT_A2B10G10R10_UNORM_PACK32) {
        swizzle = true;
    }

    for (size_t i = 0; i < m_imageCount; i++) {
        VkImageViewCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapchainFormat;

        createInfo.components.r = swizzle ? VK_COMPONENT_SWIZZLE_B : VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = swizzle ? VK_COMPONENT_SWIZZLE_R : VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(m_context.GetDevice(), &createInfo, nullptr, &m_swapchainImageViews[i]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create a swapchain image view");
    }
}

auto Swapchain::CreateSyncObjects() -> void {
    VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result;

    m_imageAvailableSemaphores.resize(k_maxFramesInFlight);
    m_inFlightFences.resize(k_maxFramesInFlight);
    for (uint32_t i = 0; i < k_maxFramesInFlight; i++) {
        result = vkCreateSemaphore(m_context.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create image available semaphores");

        result = vkCreateFence(m_context.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create in flight fences");
    }

    m_renderFinishedSemaphores.resize(m_imageCount);
    for (uint32_t i = 0; i < m_imageCount; i++) {
        auto result = vkCreateSemaphore(m_context.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create render finished semaphores");
    }

    m_imagesInFlight.resize(m_imageCount, nullptr);
}

} // namespace muon::graphics
