#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/queue.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <expected>
#include <memory>
#include <vector>

namespace muon::graphics {

constexpr uint32_t k_maxFramesInFlight = 2;

class Swapchain : NoCopy, NoMove {
public:
    struct Spec {
        const Context *context{nullptr};
        vk::Extent2D windowExtent{};
        vk::ColorSpaceKHR colorSpace{};
        vk::Format format{};
        vk::PresentModeKHR presentMode{};
        std::shared_ptr<Swapchain> oldSwapchain{nullptr};
    };

public:
    Swapchain(const Spec &spec);
    ~Swapchain();

    auto AcquireNextImage() -> std::expected<uint32_t, vk::Result>;
    auto SubmitCommandBuffers(const vk::raii::CommandBuffer &commandBuffer, uint32_t imageIndex)
        -> std::expected<void, vk::Result>;

public:
    auto Get() -> vk::raii::SwapchainKHR &;
    auto Get() const -> const vk::raii::SwapchainKHR &;

    auto GetFormat() const -> vk::Format;
    auto IsImageHdr() const -> bool;

    auto GetImageCount() const -> size_t;

    auto GetImage(size_t index) -> vk::Image &;
    auto GetImage(size_t index) const -> const vk::Image &;

    auto GetImageView(size_t index) -> vk::raii::ImageView &;
    auto GetImageView(size_t index) const -> const vk::raii::ImageView &;

    auto GetExtent() const -> vk::Extent2D;
    auto GetWidth() const -> uint32_t;
    auto GetHeight() const -> uint32_t;
    auto GetAspectRatio() const -> float;

private:
    auto CreateSwapchain(vk::Extent2D windowExtent, vk::PresentModeKHR presentMode) -> void;
    auto CreateImageViews() -> void;
    auto CreateSyncObjects() -> void;

private:
    const Context &m_context;
    const Queue &m_graphicsQueue;

    vk::raii::SwapchainKHR m_swapchain{nullptr};
    std::shared_ptr<Swapchain> m_oldSwapchain{nullptr};
    vk::Extent2D m_extent;

    uint32_t m_imageCount{0};

    vk::Format m_format;
    vk::ColorSpaceKHR m_colorSpace;
    std::vector<vk::Image> m_images{};
    std::vector<vk::raii::ImageView> m_imageViews{};

    std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores{};
    std::vector<vk::raii::Fence> m_inFlightFences{};

    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores{};
    std::vector<vk::Fence> m_imagesInFlight{};

    uint32_t m_currentFrame{0};
};

} // namespace muon::graphics
