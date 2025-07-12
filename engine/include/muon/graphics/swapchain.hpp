#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/device_context.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

constexpr uint32_t k_maxFramesInFlight = 2;

class Swapchain : NoCopy, NoMove {
public:
    struct Spec {
        const DeviceContext *device{nullptr};
        VkExtent2D windowExtent{};
        VkColorSpaceKHR colorSpace{};
        VkFormat format{};
        VkPresentModeKHR presentMode{};
        VkSwapchainKHR oldSwapchain{nullptr};
    };

public:
    Swapchain(const Spec &spec);
    ~Swapchain();

    [[nodiscard]] auto AcquireNextImage(uint32_t *imageIndex) -> VkResult;
    [[nodiscard]] auto SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) -> VkResult;

public:
    [[nodiscard]] auto GetImageCount() const -> size_t;

    [[nodiscard]] auto Get() const -> VkSwapchainKHR;
    [[nodiscard]] auto GetFormat() const -> VkFormat;
    [[nodiscard]] auto IsImageHdr() const -> bool;
    [[nodiscard]] auto GetImage(int32_t index) const -> VkImage;
    [[nodiscard]] auto GetImageView(int32_t index) const -> VkImageView;

    [[nodiscard]] auto GetExtent() const -> VkExtent2D;
    [[nodiscard]] auto GetWidth() const -> uint32_t;
    [[nodiscard]] auto GetHeight() const -> uint32_t;
    [[nodiscard]] auto GetAspectRatio() const -> float;

private:
    auto CreateSwapchain(VkExtent2D windowExtent, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain) -> void;
    auto CreateImageViews() -> void;
    auto CreateSyncObjects() -> void;

private:
    const DeviceContext &m_device;

    VkSwapchainKHR m_swapchain{nullptr};
    std::shared_ptr<Swapchain> m_oldSwapchain{nullptr};
    VkExtent2D m_swapchainExtent;

    uint32_t m_imageCount{0};

    VkFormat m_swapchainFormat;
    VkColorSpaceKHR m_swapchainColorSpace;
    std::vector<VkImage> m_swapchainImages{};
    std::vector<VkImageView> m_swapchainImageViews{};

    std::vector<VkSemaphore> m_imageAvailableSemaphores{};
    std::vector<VkFence> m_inFlightFences{};

    std::vector<VkSemaphore> m_renderFinishedSemaphores{};
    std::vector<VkFence> m_imagesInFlight{};

    uint32_t m_currentFrame{0};
};

} // namespace muon::graphics
