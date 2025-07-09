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

        [[nodiscard]] auto BeginFrame() -> VkCommandBuffer;
        auto EndFrame() -> void;

        auto RebuildSwapchain() -> void;

    public:
        [[nodiscard]] auto HasHdrSupport() const -> bool;
        [[nodiscard]] auto GetAvailableHdrSurfaceFormats() const -> const std::vector<VkSurfaceFormatKHR> &;
        [[nodiscard]] auto GetAvailableSdrSurfaceFormats() const -> const std::vector<VkSurfaceFormatKHR> &;
        [[nodiscard]] auto IsHdrEnabled() const -> bool;
        [[nodiscard]] auto GetActiveSurfaceFormat() const -> VkSurfaceFormatKHR;

        [[nodiscard]] auto GetAvailablePresentModes() const -> const std::unordered_set<VkPresentModeKHR> &;
        auto SetPresentMode(VkPresentModeKHR presentMode) -> void;

    private:
        auto ProbeSurfaceFormats() -> void;
        auto ProbePresentModes() -> void;
        auto CreateSwapchain() -> void;
        auto CreateCommandBuffers() -> void;

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
