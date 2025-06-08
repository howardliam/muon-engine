#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    namespace constants {
        constexpr uint32_t maxFramesInFlight = 2;
    }

    class Swapchain : NoCopy, NoMove {
    public:
        Swapchain();
        Swapchain(std::shared_ptr<Swapchain> previous);
        ~Swapchain();

        [[nodiscard]] VkResult AcquireNextImage(uint32_t *imageIndex);
        [[nodiscard]] VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
        [[nodiscard]] bool CompareSwapFormats(const Swapchain &other) const;

    public:
        [[nodiscard]] size_t GetImageCount() const;

        [[nodiscard]] VkSwapchainKHR GetSwapchain() const;
        [[nodiscard]] VkFormat GetSwapchainImageFormat() const;
        [[nodiscard]] VkImage GetImage(int32_t index) const;
        [[nodiscard]] VkImageView GetImageView(int32_t index) const;

        [[nodiscard]] VkExtent2D GetExtent() const;
        [[nodiscard]] uint32_t GetWidth() const;
        [[nodiscard]] uint32_t GetHeight() const;
        [[nodiscard]] float GetExtentAspectRatio() const;

    private:
        void Init();
        void CreateSwapchain();
        void CreateImageViews();
        void CreateSyncObjects();

    private:
        VkSwapchainKHR m_swapchain;
        std::shared_ptr<Swapchain> m_oldSwapchain = nullptr;
        VkExtent2D m_swapchainExtent;

        uint32_t m_imageCount;

        VkFormat m_swapchainImageFormat;
        std::vector<VkImage> m_swapchainImages{};
        std::vector<VkImageView> m_swapchainImageViews{};

        std::vector<VkSemaphore> m_imageAvailableSemaphores{};
        std::vector<VkFence> m_inFlightFences{};

        std::vector<VkSemaphore> m_renderFinishedSemaphores{};
        std::vector<VkFence> m_imagesInFlight{};

        uint32_t m_currentFrame = 0;
    };

}
