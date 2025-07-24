#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/swapchain.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <optional>
#include <unordered_set>
#include <vector>

namespace muon::graphics {

struct SurfaceFormat {
    bool isHdr{false};
    vk::Format format{vk::Format::eUndefined};
    vk::ColorSpaceKHR colorSpace{vk::ColorSpaceKHR::eSrgbNonlinear};
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

    [[nodiscard]] auto BeginFrame() -> std::optional<vk::raii::CommandBuffer *>;
    auto EndFrame() -> void;

    auto RebuildSwapchain() -> void;

public:
    auto HasHdrSupport() const -> bool;

    auto GetAvailableColorSpaces(bool hdr) const -> std::vector<vk::ColorSpaceKHR>;
    auto GetActiveSurfaceFormat() const -> const SurfaceFormat &;
    auto SetActiveSurfaceFormat(vk::ColorSpaceKHR colorSpace) const -> void;
    auto IsHdrEnabled() const -> bool;

    auto GetAvailablePresentModes() const -> const std::unordered_set<vk::PresentModeKHR> &;
    auto GetActivePresentMode() const -> const vk::PresentModeKHR &;
    auto SetActivePresentMode(vk::PresentModeKHR presentMode) const -> void;

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

    std::unordered_set<vk::PresentModeKHR> m_availablePresentModes{};
    mutable const vk::PresentModeKHR *m_activePresentMode{nullptr};

    std::unique_ptr<Swapchain> m_swapchain{nullptr};
    std::vector<vk::raii::CommandBuffer> m_commandBuffers{};

    uint32_t m_currentImageIndex{0};
    uint32_t m_currentFrameIndex{0};
    bool m_frameInProgress{false};
};

} // namespace muon::graphics
