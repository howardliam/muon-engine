#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <optional>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

namespace muon {

    class Window;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return
                graphicsFamily.has_value() &&
                computeFamily.has_value() &&
                graphicsFamily.has_value();
        }
    };

    struct SwapchainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities{};
        std::vector<vk::SurfaceFormatKHR> formats{};
        std::vector<vk::PresentModeKHR> presentModes{};
    };

    class Device : NoCopy, NoMove {
    public:
        Device(Window *window);
        ~Device();

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
        [[nodiscard]] vk::Instance instance() const;

        /**
         * @brief   gets surface.
         *
         * @return  surface.
         */
        [[nodiscard]] vk::SurfaceKHR surface() const;

        /**
         * @brief   gets physical device.
         *
         * @return  physical device.
         */
        [[nodiscard]] vk::PhysicalDevice physicalDevice() const;

        /**
         * @brief   gets logical device.
         *
         * @return  logical device.
         */
        [[nodiscard]] vk::Device device() const;

        /**
         * @brief   gets command pool.
         *
         * @return  command pool.
         */
        [[nodiscard]] vk::CommandPool commandPool() const;

        /**
         * @brief   gets graphics queue.
         *
         * @return  graphics queue.
         */
        [[nodiscard]] vk::Queue graphicsQueue() const;

        /**
            * @brief   gets compute queue.
            *
            * @return  compute queue.
            */
        [[nodiscard]] vk::Queue computeQueue() const;

        /**
         * @brief   gets present queue.
         *
         * @return  present queue.
         */
        [[nodiscard]] vk::Queue presentQueue() const;

        /**
         * @brief   gets allocator.
         *
         * @return  allocator.
         */
        [[nodiscard]] vma::Allocator allocator() const;

        /**
         * @brief   gets queue family indices.
         *
         * @return  struct containing the indices of required queue families.
         */
        [[nodiscard]] std::unique_ptr<QueueFamilyIndices> &queueFamilyIndices();

        /**
         * @brief   gets swapchain support details.
         *
         * @return  struct containing the supported swapchain options.
         */
        [[nodiscard]] SwapchainSupportDetails swapchainSupportDetails();

    private:
        void createInstance(Window *window);
        void createDebugMessenger();
        void createSurface(Window *window);
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createAllocator();
        void createCommandPool();
        void createProfiler();

        [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);
        [[nodiscard]] SwapchainSupportDetails querySwapchainSupportDetails(vk::PhysicalDevice physicalDevice);

    private:
        const std::vector<const char *> m_deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        };

        #ifdef MU_DEBUG_ENABLED
        const std::vector<const char *> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
        #endif

        vk::Instance m_instance;

        #ifdef MU_DEBUG_ENABLED
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        #endif

        vk::SurfaceKHR m_surface;
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device;

        std::unique_ptr<QueueFamilyIndices> m_queueFamilyIndices;
        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_presentQueue;

        vma::Allocator m_allocator;

        vk::CommandPool m_commandPool;
    };

}
