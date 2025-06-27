#include "muon/graphics/renderer.hpp"

#include "muon/core/assert.hpp"
#include "vulkan/vulkan_core.h"

namespace muon::gfx {

    Renderer::Renderer(const RendererSpecification &spec) : m_window(*spec.window), m_deviceContext(*spec.deviceContext) {
        ProbeSurfaceFormats();
        ProbePresentModes();

        CreateSwapchain();
        CreateCommandBuffers();
    }

    Renderer::~Renderer() {
        vkFreeCommandBuffers(
            m_deviceContext.GetDevice(),
            m_deviceContext.GetGraphicsQueue().GetCommandPool(),
            m_commandBuffers.size(),
            m_commandBuffers.data()
        );
    }

    VkCommandBuffer Renderer::BeginFrame() {
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

    void Renderer::EndFrame() {
        MU_CORE_ASSERT(m_frameInProgress, "cannot end frame if a frame has not been started");

        const auto cmd = m_commandBuffers[m_currentFrameIndex];
        vkEndCommandBuffer(cmd);
    }

    void Renderer::PresentFrame() {
        MU_CORE_ASSERT(m_frameInProgress, "cannot present frame if a frame has not been started");

        const auto cmd = m_commandBuffers[m_currentFrameIndex];
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
        m_currentFrameIndex = (m_currentFrameIndex + 1) % constants::maxFramesInFlight;
    }

    bool Renderer::HasHdrSupport() const {
        return m_hdrSupport;
    }

    const std::unordered_set<VkColorSpaceKHR> &Renderer::GetAvailableHdrColorSpaces() const {
        return m_availableHdrColorSpaces;
    }

    const std::unordered_set<VkColorSpaceKHR> &Renderer::GetAvailableSdrColorSpaces() const {
        return m_availableSdrColorSpaces;
    }

    void Renderer::SetColorSpace(VkColorSpaceKHR colorSpace) {
        MU_CORE_ASSERT(m_availableHdrColorSpaces.contains(colorSpace) || m_availableSdrColorSpaces.contains(colorSpace), "the color space must be available");

        if (m_availableHdrColorSpaces.contains(colorSpace)) {
            if (m_availableHdrFormats.contains(VK_FORMAT_A2R10G10B10_UNORM_PACK32)) {
                m_selectedFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
            } else {
                m_selectedFormat = *m_availableHdrFormats.begin();
            }
        } else if (m_availableSdrColorSpaces.contains(colorSpace)) {
            if (m_availableSdrFormats.contains(VK_FORMAT_R8G8B8A8_SRGB)) {
                m_selectedFormat = VK_FORMAT_R8G8B8A8_SRGB;
            } else {
                m_selectedFormat = *m_availableSdrFormats.begin();
            }
        }

        m_selectedColorSpace = colorSpace;
    }

    const std::unordered_set<VkPresentModeKHR> &Renderer::GetAvailablePresentModes() const {
        return m_availablePresentModes;
    }

    void Renderer::SetPresentMode(VkPresentModeKHR presentMode) {
        MU_CORE_ASSERT(m_availablePresentModes.contains(presentMode), "the present mode must be available");
        m_selectedPresentMode = presentMode;
    }

    void Renderer::ProbeSurfaceFormats() {
        uint32_t surfaceFormatCount = 0;
        auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &surfaceFormatCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface format count");
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &surfaceFormatCount, surfaceFormats.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface formats");

        bool hdrColorSpace = false;
        bool hdrFormat = false;

        for (const auto &surfaceFormat : surfaceFormats) {
            switch (surfaceFormat.colorSpace) {
                case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
                case VK_COLOR_SPACE_HDR10_ST2084_EXT:
                case VK_COLOR_SPACE_HDR10_HLG_EXT:
                case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD: {
                    hdrColorSpace = true;
                    m_availableHdrColorSpaces.insert(surfaceFormat.colorSpace);
                    break;
                }

                case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
                case VK_COLOR_SPACE_BT709_LINEAR_EXT:
                case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: {
                    m_availableSdrColorSpaces.insert(surfaceFormat.colorSpace);
                    break;
                }

                default: {
                    break;
                }
            }

            switch (surfaceFormat.format) {
                case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
                case VK_FORMAT_R16G16B16A16_SFLOAT: {
                    hdrFormat = true;
                    m_availableHdrFormats.insert(surfaceFormat.format);
                    break;
                }

                case VK_FORMAT_R8G8B8A8_UNORM: {
                    m_availableSdrFormats.insert(surfaceFormat.format);
                    break;
                }

                default: {
                    break;
                }
            }
        }

        if (hdrColorSpace && hdrFormat) {
            m_hdrSupport = true;
        }

        if (m_hdrSupport) {
            SetColorSpace(*m_availableHdrColorSpaces.begin());
        } else {
            SetColorSpace(*m_availableSdrColorSpaces.begin());
        }
    }

    void Renderer::ProbePresentModes() {
        uint32_t presentModeCount = 0;
        auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &presentModeCount, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface present modes");
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_deviceContext.GetPhysicalDevice(), m_deviceContext.GetSurface(), &presentModeCount, presentModes.data());
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
            SetPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);
        } else {
            SetPresentMode(VK_PRESENT_MODE_FIFO_KHR);
        }
    }

    void Renderer::CreateSwapchain() {
        SwapchainSpecification swapchainSpec{};
        swapchainSpec.deviceContext = &m_deviceContext;
        swapchainSpec.windowExtent = m_window.GetExtent();
        swapchainSpec.colorSpace = m_selectedColorSpace;
        swapchainSpec.presentMode = m_selectedPresentMode;
        swapchainSpec.format = m_selectedFormat;

        if (m_swapchain == nullptr) {
            swapchainSpec.oldSwapchain = nullptr;
            m_swapchain = std::make_unique<Swapchain>(swapchainSpec);
        } else {
            std::unique_ptr oldSwapChain = std::move(m_swapchain);
            swapchainSpec.oldSwapchain = oldSwapChain->GetSwapchain();
            m_swapchain = std::make_unique<Swapchain>(swapchainSpec);
        }
    }

    void Renderer::CreateCommandBuffers() {
        m_commandBuffers.resize(constants::maxFramesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level =  VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_deviceContext.GetGraphicsQueue().GetCommandPool();
        allocInfo.commandBufferCount = m_commandBuffers.size();

        auto result = vkAllocateCommandBuffers(m_deviceContext.GetDevice(), &allocInfo, m_commandBuffers.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate command buffers");
    }

}
