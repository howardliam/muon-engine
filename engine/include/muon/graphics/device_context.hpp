#pragma once

#include "muon/graphics/queue_index_helper.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <array>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::gfx {

    namespace constants {

        constexpr std::array<const char *, 4> requiredDeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        };

    }

    class DeviceContext : NoCopy, NoMove {
    public:
        DeviceContext();
        ~DeviceContext();

    public:
        [[nodiscard]] VkInstance GetInstance() const;
        [[nodiscard]] VkSurfaceKHR GetSurface() const;
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
        [[nodiscard]] VkDevice GetDevice() const;

        [[nodiscard]] QueueIndexHelper &GetQueueIndexHelper() const;

        [[nodiscard]] VmaAllocator GetAllocator() const;

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface();
        void SelectPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();

    private:
        VkInstance m_instance;

        #ifdef MU_DEBUG_ENABLED
        VkDebugUtilsMessengerEXT m_debugMessenger;
        #endif

        VkSurfaceKHR m_surface;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        std::unique_ptr<QueueIndexHelper> m_queueIndexHelper;

        VmaAllocator m_allocator;
    };

}
