#include "muon/graphics/renderer.hpp"

#include "muon/core/expect.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <algorithm>

namespace muon::graphics {

Renderer::Renderer(const Spec &spec) : m_window(*spec.window), m_context(*spec.context) {
    ProbeSurfaceFormats();
    ProbePresentModes();

    CreateSwapchain();
    CreateCommandBuffers();
}

Renderer::~Renderer() {}

auto Renderer::BeginFrame() -> std::optional<vk::raii::CommandBuffer *> {
    core::expect(!m_frameInProgress, "cannot begin frame while frame is in progress");

    auto acquireResult = m_swapchain->AcquireNextImage();
    if (!acquireResult) {
        if (acquireResult.error() == vk::Result::eErrorOutOfDateKHR) {
            CreateSwapchain();
            return {std::nullopt};
        }
    }
    m_currentImageIndex = *acquireResult;

    m_frameInProgress = true;

    auto &commandBuffer = m_commandBuffers[m_currentFrameIndex];

    vk::CommandBufferBeginInfo commandBufferBi;
    commandBuffer.begin(commandBufferBi);

    return {&commandBuffer};
}

auto Renderer::EndFrame() -> void {
    core::expect(m_frameInProgress, "cannot end frame if a frame has not been started");

    const auto &commandBuffer = m_commandBuffers[m_currentFrameIndex];
    commandBuffer.end();

    auto result = m_swapchain->SubmitCommandBuffers(commandBuffer, m_currentImageIndex);
    if (!result) {
        if (result.error() == vk::Result::eErrorOutOfDateKHR || result.error() == vk::Result::eSuboptimalKHR) {
            CreateSwapchain();
        }
    }

    m_frameInProgress = false;
    m_currentFrameIndex = (m_currentFrameIndex + 1) % k_maxFramesInFlight;
}

auto Renderer::RebuildSwapchain() -> void {
    core::expect(!m_frameInProgress, "cannot rebuild swapchain while frame is in progress");
    CreateSwapchain();
}

auto Renderer::HasHdrSupport() const -> bool { return m_hdrSupport; }

auto Renderer::GetAvailableColorSpaces(bool hdr) const -> std::vector<vk::ColorSpaceKHR> {
    std::vector<vk::ColorSpaceKHR> colorSpaces{};
    for (const auto &surfaceFormat : m_availableSurfaceFormats) {
        if (surfaceFormat.isHdr != hdr) {
            continue;
        }
        colorSpaces.push_back(surfaceFormat.colorSpace);
    }
    return colorSpaces;
}

auto Renderer::GetActiveSurfaceFormat() const -> const SurfaceFormat & { return *m_activeSurfaceFormat; }

auto Renderer::SetActiveSurfaceFormat(vk::ColorSpaceKHR colorSpace) const -> void {
    auto pred = [&colorSpace](const SurfaceFormat &surfaceFormat) { return surfaceFormat.colorSpace == colorSpace; };
    auto it = std::ranges::find_if(m_availableSurfaceFormats, pred);
    core::expect(it != m_availableSurfaceFormats.end(), "the requested color space must be available");
    m_activeSurfaceFormat = it.base();
}

auto Renderer::IsHdrEnabled() const -> bool { return m_activeSurfaceFormat->isHdr; }

auto Renderer::GetAvailablePresentModes() const -> const std::unordered_set<vk::PresentModeKHR> & {
    return m_availablePresentModes;
}

auto Renderer::GetActivePresentMode() const -> const vk::PresentModeKHR & { return *m_activePresentMode; }

auto Renderer::SetActivePresentMode(vk::PresentModeKHR presentMode) const -> void {
    core::expect(m_availablePresentModes.contains(presentMode), "the requested present mode must be available");
    m_activePresentMode = &*m_availablePresentModes.find(presentMode);
}

auto Renderer::ProbeSurfaceFormats() -> void {
    auto surfaceFormats = m_context.getPhysicalDevice().getSurfaceFormatsKHR(m_context.getSurface());
    core::expect(!surfaceFormats.empty(), "failed to get surface formats");

    for (const auto &surfaceFormat : surfaceFormats) {
        bool candidateColorSpace = false;
        bool hdrColorSpace = false;

        switch (surfaceFormat.colorSpace) {
            case vk::ColorSpaceKHR::eBt2020LinearEXT:
            case vk::ColorSpaceKHR::eHdr10St2084EXT:
            case vk::ColorSpaceKHR::eHdr10HlgEXT:
            case vk::ColorSpaceKHR::eDisplayNativeAMD: {
                hdrColorSpace = true;
                candidateColorSpace = true;
                break;
            }

            case vk::ColorSpaceKHR::eBt709NonlinearEXT:
            case vk::ColorSpaceKHR::eBt709LinearEXT:
            case vk::ColorSpaceKHR::eSrgbNonlinear: {
                candidateColorSpace = true;
                break;
            }

            default: {
                break;
            }
        }

        if (candidateColorSpace) {
            switch (surfaceFormat.format) {
                case vk::Format::eA2B10G10R10UnormPack32:
                case vk::Format::eA2R10G10B10UnormPack32:
                case vk::Format::eB8G8R8A8Srgb:
                case vk::Format::eR8G8B8A8Srgb: {
                    if (hdrColorSpace) {
                        m_hdrSupport = true;
                    }
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
    auto presentModes = m_context.getPhysicalDevice().getSurfacePresentModesKHR(m_context.getSurface());
    core::expect(!presentModes.empty(), "failed to get surface present mode count");

    for (const auto &presentMode : presentModes) {
        switch (presentMode) {
            case vk::PresentModeKHR::eMailbox:
            case vk::PresentModeKHR::eFifo:
            case vk::PresentModeKHR::eFifoRelaxed: {
                m_availablePresentModes.insert(presentMode);
                break;
            }

            default: {
                break;
            }
        }
    }

    // hard code for the moment
    if (m_availablePresentModes.contains(vk::PresentModeKHR::eMailbox)) {
        SetActivePresentMode(vk::PresentModeKHR::eMailbox);
    } else {
        SetActivePresentMode(vk::PresentModeKHR::eFifo);
    }
}

auto Renderer::CreateSwapchain() -> void {
    Swapchain::Spec swapchainSpec{};
    swapchainSpec.context = &m_context;
    swapchainSpec.windowExtent = m_window.GetExtent();
    swapchainSpec.colorSpace = m_activeSurfaceFormat->colorSpace;
    swapchainSpec.format = m_activeSurfaceFormat->format;
    swapchainSpec.presentMode = *m_activePresentMode;

    if (m_swapchain == nullptr) {
        swapchainSpec.oldSwapchain = nullptr;
        m_swapchain = std::make_unique<Swapchain>(swapchainSpec);
    } else {
        std::unique_ptr oldSwapChain = std::move(m_swapchain);
        swapchainSpec.oldSwapchain = std::move(oldSwapChain);
        m_swapchain = std::make_unique<Swapchain>(swapchainSpec);
    }
}

auto Renderer::CreateCommandBuffers() -> void {
    vk::CommandBufferAllocateInfo commandBufferAi;
    commandBufferAi.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAi.commandPool = m_context.getGraphicsQueue().GetCommandPool();
    commandBufferAi.commandBufferCount = k_maxFramesInFlight;

    auto commandBufferResult = m_context.getDevice().allocateCommandBuffers(commandBufferAi);
    core::expect(commandBufferResult, "failed to allocate command buffers");

    m_commandBuffers = std::move(*commandBufferResult);
}

} // namespace muon::graphics
