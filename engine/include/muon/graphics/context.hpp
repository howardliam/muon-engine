#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

namespace muon::gfx {

    class Context {
    public:
        Context();
        ~Context();

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

        vk::Instance m_instance;

        #ifdef MU_DEBUG_ENABLED
        vk::DebugUtilsMessengerEXT m_debugMessenger;
        #endif

        vk::SurfaceKHR m_surface;
        vk::PhysicalDevice m_physicalDevice;
        vk::Device m_device;

        vk::Queue m_graphicsQueue;
        vk::Queue m_computeQueue;
        vk::Queue m_presentQueue;

        vma::Allocator m_allocator;

        vk::CommandPool m_commandPool;
    };

}
