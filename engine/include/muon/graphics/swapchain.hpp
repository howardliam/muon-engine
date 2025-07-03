#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    namespace constants {
        constexpr uint32_t maxFramesInFlight = 2;
    }

    struct SwapchainSpecification {
        const DeviceContext *device;
        VkExtent2D windowExtent;
        VkColorSpaceKHR colorSpace;
        VkFormat format;
        VkPresentModeKHR presentMode;
        VkSwapchainKHR oldSwapchain;
    };

    class Swapchain : NoCopy, NoMove {
    public:
        Swapchain(const SwapchainSpecification &spec);
        ~Swapchain();

        [[nodiscard]] VkResult AcquireNextImage(uint32_t *imageIndex);
        [[nodiscard]] VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    public:
        [[nodiscard]] size_t GetImageCount() const;

        [[nodiscard]] VkSwapchainKHR GetSwapchain() const;
        [[nodiscard]] VkFormat GetFormat() const;
        [[nodiscard]] bool IsImageHdr() const;
        [[nodiscard]] VkImage GetImage(int32_t index) const;
        [[nodiscard]] VkImageView GetImageView(int32_t index) const;

        [[nodiscard]] VkExtent2D GetExtent() const;
        [[nodiscard]] uint32_t GetWidth() const;
        [[nodiscard]] uint32_t GetHeight() const;
        [[nodiscard]] float GetAspectRatio() const;

    private:
        void CreateSwapchain(VkExtent2D windowExtent, VkPresentModeKHR presentMode, VkSwapchainKHR oldSwapchain);
        void CreateImageViews();
        void CreateSyncObjects();

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

}
