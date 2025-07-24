#include "muon/graphics/swapchain.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"

#include <algorithm>
#include <array>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

constexpr uint64_t k_waitDuration = 30'000'000'000;

Swapchain::Swapchain(const Spec &spec)
    : m_context(*spec.context), m_graphicsQueue(m_context.GetGraphicsQueue()), m_oldSwapchain(spec.oldSwapchain),
      m_format(spec.format), m_colorSpace(spec.colorSpace) {
    CreateSwapchain(spec.windowExtent, spec.presentMode);
    CreateImageViews();
    CreateSyncObjects();

    if (spec.oldSwapchain) {
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{} from old swapchain", m_extent.width, m_extent.height);
    } else {
        MU_CORE_DEBUG("created swapchain with dimensions: {}x{}", m_extent.width, m_extent.height);
    }
}

Swapchain::~Swapchain() { MU_CORE_DEBUG("destroyed swapchain"); }

auto Swapchain::AcquireNextImage() -> std::expected<uint32_t, vk::Result> {
    auto waitResult = m_context.GetDevice().waitForFences({m_inFlightFences[m_currentFrame]}, true, k_waitDuration);
    MU_CORE_ASSERT(waitResult == vk::Result::eSuccess, "failed to wait for fences");

    vk::AcquireNextImageInfoKHR acquireInfo;
    acquireInfo.swapchain = m_swapchain;
    acquireInfo.semaphore = m_imageAvailableSemaphores[m_currentFrame];
    acquireInfo.timeout = k_waitDuration;

    auto acquireResult = m_context.GetDevice().acquireNextImage2KHR(acquireInfo);
    if (acquireResult.first != vk::Result::eSuccess) {
        return std::unexpected(acquireResult.first);
    }

    return acquireResult.second;
}

auto Swapchain::SubmitCommandBuffers(const vk::raii::CommandBuffer &commandBuffer, uint32_t imageIndex)
    -> std::expected<void, vk::Result> {
    if (m_imagesInFlight[imageIndex] != nullptr) {
        auto waitResult = m_context.GetDevice().waitForFences({m_imagesInFlight[imageIndex]}, true, k_waitDuration);
        MU_CORE_ASSERT(waitResult == vk::Result::eSuccess, "failed to wait for fences");
    }
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

    m_context.GetDevice().resetFences({m_inFlightFences[m_currentFrame]});

    vk::SubmitInfo2 submitInfo;

    vk::SemaphoreSubmitInfo waitSemaphoreSi;
    waitSemaphoreSi.semaphore = m_imageAvailableSemaphores[m_currentFrame];
    waitSemaphoreSi.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitSemaphoreSi;

    vk::CommandBufferSubmitInfo commandBufferSi;
    commandBufferSi.commandBuffer = commandBuffer;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSi;

    vk::SemaphoreSubmitInfo signalSemaphoreSi;
    signalSemaphoreSi.semaphore = m_renderFinishedSemaphores[imageIndex];
    signalSemaphoreSi.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalSemaphoreSi;

    m_graphicsQueue.Get().submit2(submitInfo);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphoreSi.semaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &(*m_swapchain);
    presentInfo.pImageIndices = &imageIndex;

    auto presentResult = m_graphicsQueue.Get().presentKHR(presentInfo);
    MU_CORE_ASSERT(presentResult == vk::Result::eSuccess, "failed to present queue");

    m_currentFrame = (m_currentFrame + 1) % k_maxFramesInFlight;

    return {};
}

auto Swapchain::Get() -> vk::raii::SwapchainKHR & { return m_swapchain; }
auto Swapchain::Get() const -> const vk::raii::SwapchainKHR & { return m_swapchain; }

auto Swapchain::GetFormat() const -> vk::Format { return m_format; }
auto Swapchain::IsImageHdr() const -> bool {
    switch (m_colorSpace) {
        case vk::ColorSpaceKHR::eBt2020LinearEXT:
        case vk::ColorSpaceKHR::eHdr10St2084EXT:
        case vk::ColorSpaceKHR::eHdr10HlgEXT:
        case vk::ColorSpaceKHR::eDisplayNativeAMD: {
            return true;
        }

        default: {
            return false;
        }
    }
}

auto Swapchain::GetImageCount() const -> size_t { return m_imageCount; }

auto Swapchain::GetImage(size_t index) -> vk::Image & { return m_images[index]; }
auto Swapchain::GetImage(size_t index) const -> const vk::Image & { return m_images[index]; }

auto Swapchain::GetImageView(size_t index) -> vk::raii::ImageView & { return m_imageViews[index]; }
auto Swapchain::GetImageView(size_t index) const -> const vk::raii::ImageView & { return m_imageViews[index]; }

auto Swapchain::GetExtent() const -> vk::Extent2D { return m_extent; }
auto Swapchain::GetWidth() const -> uint32_t { return m_extent.width; }
auto Swapchain::GetHeight() const -> uint32_t { return m_extent.height; }
auto Swapchain::GetAspectRatio() const -> float { return static_cast<float>(m_extent.width) / m_extent.height; }

auto Swapchain::CreateSwapchain(vk::Extent2D windowExtent, vk::PresentModeKHR presentMode) -> void {
    auto capabilities = m_context.GetPhysicalDevice().getSurfaceCapabilities2EXT(m_context.GetSurface());

    vk::Extent2D extent{};
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

    vk::SwapchainCreateInfoKHR swapchainCi;
    swapchainCi.surface = m_context.GetSurface();
    swapchainCi.minImageCount = minImageCount;
    swapchainCi.imageFormat = m_format;
    swapchainCi.imageColorSpace = m_colorSpace;
    swapchainCi.imageExtent = extent;
    swapchainCi.imageArrayLayers = 1;
    swapchainCi.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
    swapchainCi.imageSharingMode = vk::SharingMode::eExclusive;
    swapchainCi.preTransform = capabilities.currentTransform;
    swapchainCi.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCi.presentMode = presentMode;
    swapchainCi.clipped = true;

    std::array<uint32_t, 1> queueFamilyIndices = {m_context.GetGraphicsQueue().GetFamilyIndex()};
    swapchainCi.queueFamilyIndexCount = queueFamilyIndices.size();
    swapchainCi.pQueueFamilyIndices = queueFamilyIndices.data();

    if (m_oldSwapchain == nullptr) {
        swapchainCi.oldSwapchain = nullptr;
    }

    auto swapchainCreateResult = m_context.GetDevice().createSwapchainKHR(swapchainCi);
    MU_CORE_ASSERT(swapchainCreateResult, "failed to create the swapchain");
    m_swapchain = std::move(*swapchainCreateResult);

    m_images = m_swapchain.getImages();
    MU_CORE_ASSERT(m_images.size() > 0, "failed to get swapchain images");

    m_extent = extent;

    m_oldSwapchain = nullptr;
}

auto Swapchain::CreateImageViews() -> void {
    m_imageViews.reserve(m_imageCount);

    bool swizzle = false;
    if (m_format == vk::Format::eB8G8R8A8Srgb || m_format == vk::Format::eA2B10G10R10UnormPack32) {
        swizzle = true;
    }

    for (size_t i = 0; i < m_imageCount; i++) {
        vk::ImageViewCreateInfo imageViewCi;
        imageViewCi.image = m_images[i];
        imageViewCi.viewType = vk::ImageViewType::e2D;
        imageViewCi.format = m_format;

        imageViewCi.components.r = swizzle ? vk::ComponentSwizzle::eB : vk::ComponentSwizzle::eR;
        imageViewCi.components.g = vk::ComponentSwizzle::eG;
        imageViewCi.components.b = swizzle ? vk::ComponentSwizzle::eR : vk::ComponentSwizzle::eB;
        imageViewCi.components.a = vk::ComponentSwizzle::eA;

        imageViewCi.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageViewCi.subresourceRange.baseMipLevel = 0;
        imageViewCi.subresourceRange.levelCount = 1;
        imageViewCi.subresourceRange.baseArrayLayer = 0;
        imageViewCi.subresourceRange.layerCount = 1;

        auto imageViewResult = m_context.GetDevice().createImageView(imageViewCi);
        MU_CORE_ASSERT(imageViewResult, "failed to create a swapchain image view");

        m_imageViews[i] = std::move(*imageViewResult);
    }
}

auto Swapchain::CreateSyncObjects() -> void {
    vk::SemaphoreCreateInfo semaphoreCi;

    vk::FenceCreateInfo fenceCi;
    fenceCi.flags = vk::FenceCreateFlagBits::eSignaled;

    m_imageAvailableSemaphores.reserve(k_maxFramesInFlight);
    m_inFlightFences.reserve(k_maxFramesInFlight);
    for (uint32_t i = 0; i < k_maxFramesInFlight; i++) {
        auto semaphoreResult = m_context.GetDevice().createSemaphore(semaphoreCi);
        MU_CORE_ASSERT(semaphoreResult, "failed to create image available semaphores");
        m_imageAvailableSemaphores[i] = std::move(*semaphoreResult);

        auto fenceResult = m_context.GetDevice().createFence(fenceCi);
        MU_CORE_ASSERT(fenceResult, "failed to create in flight fences");
        m_inFlightFences[i] = std::move(*fenceResult);
    }

    m_renderFinishedSemaphores.reserve(m_imageCount);
    for (uint32_t i = 0; i < m_imageCount; i++) {
        auto semaphoreResult = m_context.GetDevice().createSemaphore(semaphoreCi);
        MU_CORE_ASSERT(semaphoreResult, "failed to create render finished semaphores");
        m_renderFinishedSemaphores[i] = std::move(*semaphoreResult);
    }

    m_imagesInFlight.resize(m_imageCount, nullptr);
}

} // namespace muon::graphics
