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

        /**
         * @brief   acquires the next image in the swapchain.
         *
         * @param   imageIndex  pointer to integer.
         *
         * @return  result of the process.
         */
        [[nodiscard]] vk::Result acquireNextImage(uint32_t *imageIndex);

        /**
         * @brief   submits command buffer for drawing.
         *
         * @param   buffers     pointer to command buffer.
         * @param   imageIndex  pointer to integer.
         *
         * @return  result of the process.
         */
        [[nodiscard]] vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *imageIndex);

        /**
         * @brief   compares swapchain image formats.
         *
         * @param   swapchain   the other swapchain to compare against.
         *
         * @return  whether they are the same.
         */
        [[nodiscard]] bool compareSwapFormats(const Swapchain &swapchain) const;

        /**
         * @brief   gets the number of images in the swapchain.
         *
         * @return  swapchain image count.
         */
        [[nodiscard]] size_t imageCount() const;

        /**
         * @brief   returns the swapchain.
         *
         * @return  swapchain handle.
         */
        [[nodiscard]] vk::SwapchainKHR getSwapchain() const;

        /**
         * @brief   returns the render pass.
         *
         * @return  render pass handle.
         */
        [[nodiscard]] vk::RenderPass getRenderPass() const;

        /**
         * @brief   returns framebuffer at image index.
         *
         * @param   index   the index of the swapchain framebuffer.
         *
         * @return  framebuffer at index.
         */
        [[nodiscard]] vk::Framebuffer getFramebuffer(int32_t index) const;

        /**
         * @brief   returns image at image index.
         *
         * @param   index   the index of the swapchain image.
         *
         * @return  image at index.
         */
        [[nodiscard]] vk::Image getImage(int32_t index) const;

        /**
         * @brief   returns image view at image index.
         *
         * @param   index   the index of the swapchain image view.
         *
         * @return  image view at index.
         */
        [[nodiscard]] vk::ImageView getImageView(int32_t index) const;

        /**
         * @brief   returns image format of the swapchain.
         *
         * @return  image format.
         */
        [[nodiscard]] vk::Format getSwapchainImageFormat() const;

        /**
         * @brief   returns the two dimensional size of the swapchain.
         *
         * @return  2d extent of the swapchain.
         */
        [[nodiscard]] vk::Extent2D getExtent() const;

        /**
         * @brief   returns the width of the swapchain images.
         *
         * @return  image width.
         */
        [[nodiscard]] uint32_t getWidth() const;

        /**
         * @brief   returns the height of the swapchain images.
         *
         * @return  image height.
         */
        [[nodiscard]] uint32_t getHeight() const;

        /**
         * @brief   returns the aspect ratio of the swapchain images.
         *
         * @return  aspect ratio.
         */
        [[nodiscard]] float getExtentAspectRatio() const;

    private:
        Device &device;
        vk::Extent2D windowExtent;

        vk::SwapchainKHR swapchain;
        std::shared_ptr<Swapchain> oldSwapchain = nullptr;
        vk::Extent2D swapchainExtent;

        vk::Format swapchainImageFormat;
        std::vector<vk::Image> swapchainImages{};
        std::vector<vk::ImageView> swapchainImageViews{};

        vk::Format depthImageFormat;
        std::vector<vk::Image> depthImages{};
        std::vector<vma::Allocation> depthImageAllocations{};
        std::vector<vk::ImageView> depthImageViews{};

        vk::RenderPass renderPass;

        std::vector<vk::Framebuffer> swapchainFramebuffers{};

        std::vector<vk::Semaphore> imageAvailableSemaphores{};
        std::vector<vk::Semaphore> renderFinishedSemaphores{};
        std::vector<vk::Fence> inFlightFences{};
        std::vector<vk::Fence> imagesInFlight{};

        int32_t currentFrame{0};

        /**
         * @brief   initialises the swapchain.
         */
        void init();

        /**
         * @brief   creates the swapchain.
         */
        void createSwapchain();

        /**
         * @brief   creates all the image views.
         */
        void createImageViews();

        /**
         * @brief   creates the depth buffer resources.
         */
        void createDepthResources();

        /**
         * @brief   creates the render pass.
         */
        void createRenderPass();

        /**
         * @brief   creates all the framebuffers.
         */
        void createFramebuffers();

        /**
         * @brief   creates the synchronisation objects.
         */
        void createSyncObjects();

    };

}
