#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/swapchain.hpp"

#include <unordered_set>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

struct SurfaceFormat {
    bool isHdr{false};
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkColorSpaceKHR colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
};

class Renderer : NoCopy, NoMove {
public:
    struct Spec {
        const Window *window{nullptr};
        const Context *context{nullptr};
    };

public:
    Renderer(const Spec &spec);
    ~Renderer();

    [[nodiscard]] auto BeginFrame() -> VkCommandBuffer;
    auto EndFrame() -> void;

    auto RebuildSwapchain() -> void;

public:
    [[nodiscard]] auto HasHdrSupport() const -> bool;

    [[nodiscard]] auto GetAvailableColorSpaces(bool hdr) const -> std::vector<VkColorSpaceKHR>;
    [[nodiscard]] auto GetActiveSurfaceFormat() const -> const SurfaceFormat &;
    auto SetActiveSurfaceFormat(VkColorSpaceKHR colorSpace) const -> void;
    [[nodiscard]] auto IsHdrEnabled() const -> bool;

    [[nodiscard]] auto GetAvailablePresentModes() const -> const std::unordered_set<VkPresentModeKHR> &;
    [[nodiscard]] auto GetActivePresentMode() const -> const VkPresentModeKHR &;
    auto SetActivePresentMode(VkPresentModeKHR presentMode) const -> void;

private:
    auto ProbeSurfaceFormats() -> void;
    auto ProbePresentModes() -> void;
    auto CreateSwapchain() -> void;
    auto CreateCommandBuffers() -> void;

private:
    const Window &m_window;
    const Context &m_context;

    bool m_hdrSupport{false};
    std::vector<SurfaceFormat> m_availableSurfaceFormats{};
    mutable const SurfaceFormat *m_activeSurfaceFormat{nullptr};

    std::unordered_set<VkPresentModeKHR> m_availablePresentModes{};
    mutable const VkPresentModeKHR *m_activePresentMode{nullptr};

    std::unique_ptr<Swapchain> m_swapchain{nullptr};
    std::vector<VkCommandBuffer> m_commandBuffers{};

    uint32_t m_currentImageIndex{0};
    uint32_t m_currentFrameIndex{0};
    bool m_frameInProgress{false};
};

} // namespace muon::graphics
