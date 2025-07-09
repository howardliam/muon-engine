#include "muon/graphics/renderer.hpp"

#include "muon/core/assert.hpp"
#include "vulkan/vulkan_core.h"
#include <algorithm>

namespace muon::graphics {

    Renderer::Renderer(const Spec &spec) : m_window(*spec.window), m_device(*spec.device) {
        ProbeSurfaceFormats();
        ProbePresentModes();

        CreateSwapchain();
        CreateCommandBuffers();
    }

    Renderer::~Renderer() {
        vkFreeCommandBuffers(
            m_device.GetDevice(),
            m_device.GetGraphicsQueue().GetCommandPool(),
            m_commandBuffers.size(),
            m_commandBuffers.data()
        );
    }

    auto Renderer::BeginFrame() -> VkCommandBuffer {
        MU_CORE_ASSERT(!m_frameInProgress, "cannot begin frame while frame is in progress");

        auto result = m_swapchain->AcquireNextImage(&m_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            CreateSwapchain();
            return nullptr;
        }
        MU_CORE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "failed to acquire next swapchain image");

        m_frameInProgress = true;

        const auto cmd = m_commandBuffers[m_currentFrameIndex];

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        result = vkBeginCommandBuffer(cmd, &beginInfo);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to begin recording command buffer");

        return cmd;
    }

    auto Renderer::EndFrame() -> void{
        MU_CORE_ASSERT(m_frameInProgress, "cannot end frame if a frame has not been started");

        const auto cmd = m_commandBuffers[m_currentFrameIndex];
        vkEndCommandBuffer(cmd);

        auto result = m_swapchain->SubmitCommandBuffers(&cmd, &m_currentImageIndex);
        MU_CORE_ASSERT(
            result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR ||
            result == VK_SUCCESS,
            "failed to present swapchain image"
        );
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            CreateSwapchain();
        }

        m_frameInProgress = false;
        m_currentFrameIndex = (m_currentFrameIndex + 1) % k_maxFramesInFlight;
    }

    auto Renderer::RebuildSwapchain() -> void {
        MU_CORE_ASSERT(!m_frameInProgress, "cannot rebuild swapchain while frame is in progress");
        CreateSwapchain();
    }

    auto Renderer::HasHdrSupport() const -> bool {
        return m_hdrSupport;
    }

    auto Renderer::GetAvailableColorSpaces(bool hdr) const -> std::vector<VkColorSpaceKHR> {
        std::vector<VkColorSpaceKHR> colorSpaces{};
        for (const auto &surfaceFormat : m_availableSurfaceFormats) {
            if (surfaceFormat.isHdr != hdr) { continue; }
            colorSpaces.push_back(surfaceFormat.colorSpace);
        }
        return colorSpaces;
    }

    auto Renderer::GetActiveSurfaceFormat() const -> const SurfaceFormat & {
        return *m_activeSurfaceFormat;
    }

    auto Renderer::SetActiveSurfaceFormat(VkColorSpaceKHR colorSpace) const -> void {
        auto pred = [&colorSpace](const SurfaceFormat &surfaceFormat) { return surfaceFormat.colorSpace == colorSpace; };
        auto it = std::ranges::find_if(m_availableSurfaceFormats, pred);
        MU_CORE_ASSERT(it != m_availableSurfaceFormats.end(), "the requested color space must be available");
        m_activeSurfaceFormat = it.base();
    }

    auto Renderer::IsHdrEnabled() const -> bool {
        return m_activeSurfaceFormat->isHdr;
    }

    auto Renderer::GetAvailablePresentModes() const -> const std::unordered_set<VkPresentModeKHR> &{
        return m_availablePresentModes;
    }

    auto Renderer::GetActivePresentMode() const -> const VkPresentModeKHR & {
        return *m_activePresentMode;
    }

    auto Renderer::SetActivePresentMode(VkPresentModeKHR presentMode) const -> void {
        MU_CORE_ASSERT(m_availablePresentModes.contains(presentMode), "the requested present mode must be available");
        m_activePresentMode = &*m_availablePresentModes.find(presentMode);
    }

    auto Renderer::ProbeSurfaceFormats() -> void {
        uint32_t surfaceFormatCount = 0;
        auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.GetPhysicalDevice(), m_device.GetSurface(), &surfaceFormatCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface format count");
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_device.GetPhysicalDevice(), m_device.GetSurface(), &surfaceFormatCount, surfaceFormats.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface formats");

        for (const auto &surfaceFormat : surfaceFormats) {
            bool candidateColorSpace = false;
            bool hdrColorSpace = false;

            switch (surfaceFormat.colorSpace) {
                case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
                case VK_COLOR_SPACE_HDR10_ST2084_EXT:
                case VK_COLOR_SPACE_HDR10_HLG_EXT:
                case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: {
                    hdrColorSpace = true;
                    candidateColorSpace = true;
                    break;
                }

                case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
                case VK_COLOR_SPACE_BT709_LINEAR_EXT:
                case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: {
                    candidateColorSpace = true;
                    break;
                }

                default: {
                    break;
                }
            }

            if (candidateColorSpace) {
                switch (surfaceFormat.format) {
                    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
                    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                    case VK_FORMAT_R8G8B8A8_SRGB:
                    case VK_FORMAT_B8G8R8A8_SRGB: {
                        if (hdrColorSpace) { m_hdrSupport = true; }
                        m_availableSurfaceFormats.emplace_back(hdrColorSpace, surfaceFormat.format, surfaceFormat.colorSpace);
                        break;
                    }

                    default: {
                        break;
                    }
                }
            }
        }

        if (m_hdrSupport) {
            auto it = std::ranges::find_if(m_availableSurfaceFormats, [](const SurfaceFormat &format) { return format.isHdr; });
            if (it != m_availableSurfaceFormats.end()) {
                m_activeSurfaceFormat = it.base();
            }
        } else {
            m_activeSurfaceFormat = &m_availableSurfaceFormats.front();
        }
    }

    auto Renderer::ProbePresentModes() -> void {
        uint32_t presentModeCount = 0;
        auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.GetPhysicalDevice(), m_device.GetSurface(), &presentModeCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present modes");
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_device.GetPhysicalDevice(), m_device.GetSurface(), &presentModeCount, presentModes.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present mode count");

        for (const auto &presentMode : presentModes) {
            switch (presentMode) {
                case VK_PRESENT_MODE_MAILBOX_KHR:
                case VK_PRESENT_MODE_FIFO_KHR:
                case VK_PRESENT_MODE_FIFO_RELAXED_KHR: {
                    m_availablePresentModes.insert(presentMode);
                    break;
                }

                default: {
                    break;
                }
            }
        }

        // hard code for the moment
        if (m_availablePresentModes.contains(VK_PRESENT_MODE_MAILBOX_KHR)) {
            SetActivePresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
        } else {
            SetActivePresentMode(VK_PRESENT_MODE_FIFO_KHR);
        }
    }

    auto Renderer::CreateSwapchain() -> void {
        Swapchain::Spec swapchainSpec{};
        swapchainSpec.device = &m_device;
        swapchainSpec.windowExtent = m_window.GetExtent();
        swapchainSpec.colorSpace = m_activeSurfaceFormat->colorSpace;
        swapchainSpec.format = m_activeSurfaceFormat->format;
        swapchainSpec.presentMode = *m_activePresentMode;

        if (m_swapchain == nullptr) {
            swapchainSpec.oldSwapchain = nullptr;
            m_swapchain = std::make_unique<Swapchain>(swapchainSpec);
        } else {
            std::unique_ptr oldSwapChain = std::move(m_swapchain);
            swapchainSpec.oldSwapchain = oldSwapChain->Get();
            m_swapchain = std::make_unique<Swapchain>(swapchainSpec);
        }
    }

    auto Renderer::CreateCommandBuffers() -> void {
        m_commandBuffers.resize(k_maxFramesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level =  VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.GetGraphicsQueue().GetCommandPool();
        allocInfo.commandBufferCount = m_commandBuffers.size();

        auto result = vkAllocateCommandBuffers(m_device.GetDevice(), &allocInfo, m_commandBuffers.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate command buffers");
    }

}
