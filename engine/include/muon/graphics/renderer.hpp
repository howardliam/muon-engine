#pragma once

#include "muon/core/window.hpp"
#include "muon/graphics/device_context.hpp"
#include "muon/graphics/swapchain.hpp"
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    struct RendererSpecification {
        const Window *window;
        const DeviceContext *deviceContext;
    };

    class Renderer {
    public:
        Renderer(const RendererSpecification &spec);
        ~Renderer();

        [[nodiscard]] VkCommandBuffer BeginFrame();
        void EndFrame();
        void PresentFrame();

        void RebuildSwapchain();

    public:
        [[nodiscard]] bool HasHdrSupport() const;
        [[nodiscard]] const std::unordered_set<VkColorSpaceKHR> &GetAvailableHdrColorSpaces() const;
        [[nodiscard]] const std::unordered_set<VkColorSpaceKHR> &GetAvailableSdrColorSpaces() const;
        void SetColorSpace(VkColorSpaceKHR colorSpace);

        [[nodiscard]] const std::unordered_set<VkPresentModeKHR> &GetAvailablePresentModes() const;
        void SetPresentMode(VkPresentModeKHR presentMode);

    private:
        void ProbeSurfaceFormats();
        void ProbePresentModes();
        void CreateSwapchain();
        void CreateCommandBuffers();

    private:
        const Window &m_window;
        const DeviceContext &m_deviceContext;

        bool m_hdrSupport = false;
        std::unordered_set<VkColorSpaceKHR> m_availableHdrColorSpaces{};
        std::unordered_set<VkColorSpaceKHR> m_availableSdrColorSpaces{};
        VkColorSpaceKHR m_selectedColorSpace;
        std::unordered_set<VkFormat> m_availableHdrFormats{};
        std::unordered_set<VkFormat> m_availableSdrFormats{};
        VkFormat m_selectedFormat;

        std::unordered_set<VkPresentModeKHR> m_availablePresentModes{};
        VkPresentModeKHR m_selectedPresentMode;

        std::unique_ptr<Swapchain> m_swapchain = nullptr;
        std::vector<VkCommandBuffer> m_commandBuffers{};

        uint32_t m_currentImageIndex = 0;
        uint32_t m_currentFrameIndex = 0;
        bool m_frameInProgress = false;
    };

}
