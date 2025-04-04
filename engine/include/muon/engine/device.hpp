#pragma once

#include <optional>
#include <cstdint>
#include <vector>
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
        Device(Window &window);
        ~Device();

        Device(const Device &) = delete;
        Device &operator=(const Device& ) = delete;
        Device(const Device &&) = delete;
        Device &operator=(const Device &&) = delete;

        [[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);

        void createImage(const vk::ImageCreateInfo &imageInfo, vk::MemoryPropertyFlags properties, vk::Image &image, vma::Allocation &allocation);

        /**
         * @brief   Gets Vulkan instance.
         *
         * @return  Vulkan instance.
        */
        [[nodiscard]] vk::Instance getInstance();

        /**
         * @brief   Gets Vulkan surface.
         *
         * @return  Vulkan surface.
        */
        [[nodiscard]] vk::SurfaceKHR getSurface();

        /**
         * @brief   Gets physical device.
         *
         * @return  physical device.
        */
        [[nodiscard]] vk::PhysicalDevice getPhysicalDevice();

        /**
         * @brief   Gets logical device.
         *
         * @return  logical device.
        */
        [[nodiscard]] vk::Device getDevice();

        /**
         * @brief   Gets graphics queue.
         *
         * @return  graphics queue.
        */
        [[nodiscard]] vk::Queue getGraphicsQueue();

        /**
         * @brief   Gets present queue.
         *
         * @return  present queue.
        */
        [[nodiscard]] vk::Queue getPresentQueue();

        /**
         * @brief   Gets allocator.
         *
         * @return  allocator.
        */
        [[nodiscard]] vma::Allocator getAllocator();

        /**
         * @brief   Gets queue family indices.
         *
         * @return  struct containing the indices of required queue families.
        */
        [[nodiscard]] QueueFamilyIndices getQueueFamilyIndices();

        /**
         * @brief   Gets swapchain support details.
         *
         * @return  struct containing the supported swapchain options.
        */
        [[nodiscard]] SwapchainSupportDetails getSwapchainSupportDetails();

        [[nodiscard]] vk::CommandPool getCommandPool() const;

    private:
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
         * @brief   Creates Vulkan instance.
        */
        void createInstance();

        /**
         * @brief   Creates debug messenger.
        */
        void createDebugMessenger();

        /**
         * @brief   Creates window surface.
        */
        void createSurface();

        /**
         * @brief   Selects the physical device.
        */
        void selectPhysicalDevice();

        /**
         * @brief   Creates logical device.
        */
        void createLogicalDevice();

        /**
         * @brief   Creates GPU memory allocator.
        */
        void createAllocator();

        void createCommandPool();

        /**
         * @brief   Finds queue family indices for the physical device.
         *
         * @param   physicalDevice  the physical device to query queue index information about.
         *
         * @return  struct containing the indices of required queue families.
        */
        [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);

        /**
         * @brief   Queries the physical device for supported swapchain features.
         *
         * @param   physicalDevice  the physical device to query swapchain information.
         *
         * @return  struct containing the supported swapchain options.
        */
        [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice physicalDevice);
    };

}
