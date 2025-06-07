#pragma once

#include "muon/graphics/queue.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::gfx {

    class Context : NoCopy, NoMove {
    public:
        Context();
        ~Context();

    public:
        [[nodiscard]] VkInstance GetInstance() const;
        [[nodiscard]] VkSurfaceKHR GetSurface() const;
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
        [[nodiscard]] VkDevice GetDevice() const;

        [[nodiscard]] QueueFamilyIndices &GetQueueIndices() const;
        [[nodiscard]] Queue &GetGraphicsQueue() const;
        [[nodiscard]] Queue &GetPresentQueue() const;
        [[nodiscard]] Queue &GetComputeQueue() const;
        [[nodiscard]] Queue &GetTransferQueue() const;

        [[nodiscard]] VmaAllocator GetAllocator() const;

        [[nodiscard]] VkCommandPool GetCommandPool() const;

    private:
        void CreateInstance();
        void CreateDebugMessenger();
        void CreateSurface();
        void SelectPhysicalDevice();
        void CreateLogicalDevice();
        void CreateQueues();
        void CreateAllocator();
        void CreateProfiler();

    private:
        VkInstance m_instance;

        #ifdef MU_DEBUG_ENABLED
        VkDebugUtilsMessengerEXT m_debugMessenger;
        #endif

        VkSurfaceKHR m_surface;
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        std::unique_ptr<QueueFamilyIndices> m_queueFamilyIndices;
        std::unique_ptr<Queue> m_graphicsQueue;
        std::unique_ptr<Queue> m_presentQueue;
        std::unique_ptr<Queue> m_computeQueue;
        std::unique_ptr<Queue> m_transferQueue;

        VmaAllocator m_allocator;
    };

}
