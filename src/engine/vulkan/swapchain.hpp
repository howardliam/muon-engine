#pragma once

#include <vulkan/vulkan.hpp>

#include "core/logging.hpp"
#include "engine/vulkan/device.hpp"

namespace muon::engine {


    class Swapchain {
    public:
        static constexpr uint32_t maxFramesInFlight = 2;

        Swapchain(logging::Logger logger, Device &device, vk::Extent2D windowExtent);
        ~Swapchain();

        /* Getters & Setters */
        [[nodiscard]] size_t imageCount() { return swapchainImages.size(); }
        [[nodiscard]] vk::SwapchainKHR getSwapchain() { return swapchain; }

        [[nodiscard]] vk::RenderPass getRenderPass() { return renderPass; }

    private:
        logging::Logger logger;
        Device &device;
        vk::Extent2D windowExtent;

        vk::SwapchainKHR swapchain;
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

        /* Set up swapchain */
        void init();
        void createSwapchain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
    };

}
