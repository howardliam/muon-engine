#pragma once

#include "muon/engine/device.hpp"
#include <cstddef>
#include <memory>
#include <vector>
#include <vk_mem_alloc_handles.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    namespace constants {
        constexpr uint32_t maxFramesInFlight = 2;
    }

    class Swapchain {
    public:
        Swapchain(Device &device, vk::Extent2D windowExtent);
        Swapchain(Device &device, vk::Extent2D windowExtent, std::shared_ptr<Swapchain> previous);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain &operator=(const Swapchain &) = delete;

        [[nodiscard]] size_t imageCount() const;
        [[nodiscard]] vk::SwapchainKHR getSwapchain() const;
        [[nodiscard]] vk::RenderPass getRenderPass() const;
        [[nodiscard]] vk::Framebuffer getFramebuffer(int32_t index) const;
        [[nodiscard]] vk::ImageView getImageView(int32_t index) const;
        [[nodiscard]] vk::Format getSwapchainImageFormat() const;
        [[nodiscard]] vk::Extent2D getExtent() const;
        [[nodiscard]] uint32_t getWidth() const;
        [[nodiscard]] uint32_t getHeight() const;
        [[nodiscard]] float getExtentAspectRatio() const;

    private:
        Device &device;
        vk::Extent2D windowExtent;

        vk::SwapchainKHR swapchain;
        std::shared_ptr<Swapchain> oldSwapchain;
        vk::Extent2D swapchainExtent;

        vk::Format swapchainImageFormat;
        std::vector<vk::Image> swapchainImages;
        std::vector<vk::ImageView> swapchainImageViews;

        vk::Format depthImageFormat;
        std::vector<vk::Image> depthImages;
        std::vector<vma::Allocation> depthImageAllocations;
        std::vector<vk::ImageView> depthImageViews;

        vk::RenderPass renderPass;

        std::vector<vk::Framebuffer> swapchainFramebuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;

        void init();
        void createSwapchain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

    };

}
