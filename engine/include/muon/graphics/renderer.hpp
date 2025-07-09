#pragma once

#include "muon/core/window.hpp"
#include "muon/graphics/device_context.hpp"
#include "muon/graphics/swapchain.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <limits>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    class Renderer : NoCopy, NoMove {
    public:
        struct Spec {
            const Window *window{nullptr};
            const DeviceContext *device{nullptr};
        };

    public:
        Renderer(const Spec &spec);
        ~Renderer();

        [[nodiscard]] VkCommandBuffer BeginFrame();
        void EndFrame();

        void RebuildSwapchain();

    public:
        [[nodiscard]] bool HasHdrSupport() const;
        [[nodiscard]] const std::vector<VkSurfaceFormatKHR> &GetAvailableHdrSurfaceFormats() const;
        [[nodiscard]] const std::vector<VkSurfaceFormatKHR> &GetAvailableSdrSurfaceFormats() const;
        [[nodiscard]] bool IsHdrEnabled() const;
        [[nodiscard]] VkSurfaceFormatKHR GetActiveSurfaceFormat() const;

        [[nodiscard]] const std::unordered_set<VkPresentModeKHR> &GetAvailablePresentModes() const;
        void SetPresentMode(VkPresentModeKHR presentMode);

    private:
        void ProbeSurfaceFormats();
        void ProbePresentModes();
        void CreateSwapchain();
        void CreateCommandBuffers();

    private:
        const Window &m_window;
        const DeviceContext &m_device;

        bool m_hdrSupport{false};
        std::vector<VkSurfaceFormatKHR> m_availableHdrSurfaceFormats{};
        std::vector<VkSurfaceFormatKHR> m_availableSdrSurfaceFormats{};
        bool m_hdrEnabled{false};
        uint32_t m_activeSurfaceFormat{std::numeric_limits<uint32_t>().max()};

        std::unordered_set<VkPresentModeKHR> m_availablePresentModes{};
        VkPresentModeKHR m_selectedPresentMode{VK_PRESENT_MODE_FIFO_KHR};

        std::unique_ptr<Swapchain> m_swapchain{nullptr};
        std::vector<VkCommandBuffer> m_commandBuffers{};

        uint32_t m_currentImageIndex{0};
        uint32_t m_currentFrameIndex{0};
        bool m_frameInProgress{false};
    };

}
