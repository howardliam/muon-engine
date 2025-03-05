#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan-memory-allocator-hpp/vk_mem_alloc.hpp>

#include "core/logging.hpp"
#include "core/window.hpp"

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
        Device(logging::Logger logger, window::Window &window, toml::table &config);
        ~Device();

        /* Getters & Setters */
        [[nodiscard]] vk::Instance getInstance() { return instance; }
        [[nodiscard]] vk::SurfaceKHR getSurface() { return surface; }
        [[nodiscard]] vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
        [[nodiscard]] vk::Device getDevice() { return device; }
        [[nodiscard]] vk::Queue getGraphicsQueue() { return graphicsQueue; }
        [[nodiscard]] vk::Queue getPresentQueue() { return presentQueue; }
        [[nodiscard]] vma::Allocator getAllocator() { return allocator; }

        [[nodiscard]] SwapchainSupportDetails getSwapchainSupport() { return querySwapchainSupport(physicalDevice); }
        [[nodiscard]] QueueFamilyIndices getQueueFamilyIndices() { return findQueueFamilies(physicalDevice); }

        /* Other methods */
        [[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        void createImage(const vk::ImageCreateInfo &imageInfo, vk::MemoryPropertyFlags properties, vk::Image &image, vma::Allocation &allocation);

    private:
        logging::Logger logger;
        window::Window &window;

        bool enableValidationLayers = false;
        const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
        const std::vector<const char *> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;
        vk::SurfaceKHR surface;
        vk::PhysicalDevice physicalDevice;
        vk::Device device;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        vma::Allocator allocator;

        /* Set up vulkan */
        void createInstance();
        void createDebugMessenger();
        void createSurface();
        void selectPhysicalDevice();
        void createLogicalDevice();
        void createAllocator();

        [[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice physicalDevice);
        [[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice physicalDevice);
    };

}
