#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::gfx {

    class Context {
    public:
        Context();
        ~Context();

        [[nodiscard]] VkInstance GetInstance() const;
        [[nodiscard]] VkSurfaceKHR GetSurface() const;
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
        [[nodiscard]] VkDevice GetDevice() const;
        [[nodiscard]] VkQueue GetGraphicsQueue() const;
        [[nodiscard]] VkQueue GetComputeQueue() const;
        [[nodiscard]] VkQueue GetPresentQueue() const;
        [[nodiscard]] VmaAllocator GetAllocator() const;
        [[nodiscard]] VkCommandPool GetCommandPool() const;

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface();
        void SelectPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();
        void CreateCommandPool();
        void CreateProfiler();

    private:
        const std::vector<const char *> m_deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        };

        VkInstance m_instance;

        #ifdef MU_DEBUG_ENABLED
        VkDebugUtilsMessengerEXT m_debugMessenger;
        #endif

        VkSurfaceKHR m_surface;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_presentQueue;

        VmaAllocator m_allocator;

        VkCommandPool m_commandPool;
    };

}
