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

        [[nodiscard]] vk::Result acquireNextImage(uint32_t *imageIndex);
        [[nodiscard]] vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex);
        [[nodiscard]] bool compareSwapFormats(const Swapchain &swapchain) const;

        /**
         * @brief   Gets the number of images in the swapchain.
         *
         * @return  swapchain image count.
        */
        [[nodiscard]] size_t imageCount() const;

        /**
         * @brief   Returns the swapchain.
         *
         * @return  swapchain handle.
        */
        [[nodiscard]] vk::SwapchainKHR getSwapchain() const;

        /**
         * @brief   Returns the render pass.
         *
         * @return  render pass handle.
        */
        [[nodiscard]] vk::RenderPass getRenderPass() const;

        /**
         * @brief   Returns framebuffer at image index.
         *
         * @param   index   the index of the swapchain framebuffer.
         *
         * @return  framebuffer at index.
        */
        [[nodiscard]] vk::Framebuffer getFramebuffer(int32_t index) const;

        /**
         * @brief   Returns image view at image index.
         *
         * @param   index   the index of the swapchain image view.
         *
         * @return  image view at index.
        */
        [[nodiscard]] vk::ImageView getImageView(int32_t index) const;

        /**
         * @brief   Returns image format of the swapchain.
         *
         * @return  image format.
        */
        [[nodiscard]] vk::Format getSwapchainImageFormat() const;

        /**
         * @brief   Returns the two dimensional size of the swapchain.
         *
         * @return  2d extent of the swapchain.
        */
        [[nodiscard]] vk::Extent2D getExtent() const;

        /**
         * @brief   Returns the width of the swapchain images.
         *
         * @return  image width.
        */
        [[nodiscard]] uint32_t getWidth() const;

        /**
         * @brief   Returns the height of the swapchain images.
         *
         * @return  image height.
        */
        [[nodiscard]] uint32_t getHeight() const;

        /**
         * @brief   Returns the aspect ratio of the swapchain images.
         *
         * @return  aspect ratio.
        */
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
        std::vector<vk::Fence> imagesInFlight;

        int32_t currentFrame = 0;

        /**
         * @brief   Initialises the swapchain.
        */
        void init();

        /**
         * @brief   Creates the swapchain.
        */
        void createSwapchain();

        /**
         * @brief   Creates all the image views.
        */
        void createImageViews();

        /**
         * @brief   Creates the depth buffer resources.
        */
        void createDepthResources();

        /**
         * @brief   Creates the render pass.
        */
        void createRenderPass();

        /**
         * @brief   Creates all the framebuffers.
        */
        void createFramebuffers();

        /**
         * @brief   Creates the synchronisation objects.
        */
        void createSyncObjects();

    };

}
