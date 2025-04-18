#pragma once

#include <memory>
#include <optional>
#include <cstdint>
#include <vector>
#include "muon/misc/logger.hpp"
#include "muon/engine/window.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

namespace muon::engine {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && graphicsFamily.has_value();
        }
    };

    struct SwapchainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    class Device {
    public:
        Device(std::shared_ptr<misc::ILogger> logger, Window &window);
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device& ) = delete;
        Device(const Device &&) = delete;
        Device &operator=(const Device &&) = delete;

        /**
         * @brief   finds supported image format.
         *
         * @param   candidates  a vector of image formats.
         * @param   tiling      the preferred image tiling method.
         * @param   features    the required features of the image format.
         *
         * @return  the suitable image format from the candidates.
         */
        [[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        /**
         * @brief   creates a command buffer for a single task, not part of the rendering pipeline.
         *
         * @return  command buffer instance.
         */
        [[nodiscard]] vk::CommandBuffer beginSingleTimeCommands();

        /**
         * @brief   ends an active command buffer, used to end <beginSingleTimeCommands>"()".
         */
        void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

        /**
         * @brief   copies one buffer into another.
         *
         * @param   src     the source buffer.
         * @param   dst     the destination buffer.
         * @param   size    the size of the buffer contents.
         */
        void copyBuffer(vk::Buffer src, vk::Buffer dest, vk::DeviceSize size);

        /**
         * @brief   copies contents of a buffer into an image.
         *
         * @param   buffer      the buffer to copy from.
         * @param   image       the image to copy into.
         * @param   width       the width of the image.
         * @param   height      the height of the image.
         * @param   layerCount  how many layers the image has.
         */
        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount);

        /**
         * @brief   copies contents of an image into a buffer.
         *
         * @param   image       the image to copy from.
         * @param   buffer      the buffer to copy into.
         * @param   width       the width of the image.
         * @param   height      the height of the image.
         * @param   layerCount  how many layers the image has.
         */
        void copyImageToBuffer(vk::Image image, vk::Buffer buffer, uint32_t width, uint32_t height, uint32_t layerCount);

        /**
         * @brief   allocates a new buffer.
         *
         * @param   size        the size of the buffer.
         * @param   usage       the usage flags for the buffer.
         * @param   memoryUsage the memory usage for the buffer.
         * @param   buffer      buffer handle.
         * @param   allocation  allocation handle.
         */
        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage, vk::Buffer &buffer, vma::Allocation &allocation);

        /**
         * @brief   allocates a new image.
         *
         * @param   imageInfo   the information to create the image.
         * @param   properties  the memory properties required for the image.
         * @param   memoryUsage the memory usage for the image.
         * @param   image       image handle.
         * @param   allocation  allocation handle.
         */
        void createImage(const vk::ImageCreateInfo &imageInfo, vk::MemoryPropertyFlags properties, vma::MemoryUsage memoryUsage, vk::Image &image, vma::Allocation &allocation);

        /**
         * @brief   gets instance.
         *
         * @return  instance.
         */
        [[nodiscard]] vk::Instance getInstance() const;

        /**
         * @brief   gets surface.
         *
         * @return  surface.
         */
        [[nodiscard]] vk::SurfaceKHR getSurface() const;

        /**
         * @brief   gets physical device.
         *
         * @return  physical device.
         */
        [[nodiscard]] vk::PhysicalDevice getPhysicalDevice() const;

        /**
         * @brief   gets logical device.
         *
         * @return  logical device.
         */
        [[nodiscard]] vk::Device getDevice() const;

        /**
         * @brief   gets command pool.
         *
         * @return  command pool.
         */
        [[nodiscard]] vk::CommandPool getCommandPool() const;

        /**
         * @brief   gets graphics queue.
         *
         * @return  graphics queue.
         */
        [[nodiscard]] vk::Queue getGraphicsQueue() const;

        /**
         * @brief   gets present queue.
         *
         * @return  present queue.
         */
        [[nodiscard]] vk::Queue getPresentQueue() const;

        /**
         * @brief   gets allocator.
         *
         * @return  allocator.
         */
        [[nodiscard]] vma::Allocator getAllocator() const;

        /**
         * @brief   gets queue family indices.
         *
         * @return  struct containing the indices of required queue families.
         */
        [[nodiscard]] QueueFamilyIndices getQueueFamilyIndices();

        /**
         * @brief   gets swapchain support details.
         *
         * @return  struct containing the supported swapchain options.
         */
        [[nodiscard]] SwapchainSupportDetails getSwapchainSupportDetails();

    private:
        std::shared_ptr<misc::ILogger> logger;

        Window &window;

        bool enableValidationLayers = true;
        const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char *> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;
        vk::SurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;
        vk::CommandPool commandPool;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        vma::Allocator allocator;

        /**
         * @brief   creates instance.
         */
        void createInstance();

        /**
         * @brief   creates debug messenger.
         */
        void createDebugMessenger();

        /**
         * @brief   creates window surface.
         */
        void createSurface();

        /**
         * @brief   selects the physical device.
         */
        void selectPhysicalDevice();

        /**
         * @brief   creates logical device.
         */
        void createLogicalDevice();

        /**
         * @brief   creates GPU memory allocator.
         */
        void createAllocator();

        /**
         * @brief   creates the command pool for making command buffers.
         */
        void createCommandPool();

        /**
         * @brief   finds queue family indices for the physical device.
         *
         * @param   physicalDevice  the physical device to query queue index information about.
         *
         * @return  struct containing the indices of required queue families.
         */
        [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);

        /**
         * @brief   queries the physical device for supported swapchain features.
         *
         * @param   physicalDevice  the physical device to query swapchain information.
         *
         * @return  struct containing the supported swapchain options.
         */
        [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice physicalDevice);
    };

}
