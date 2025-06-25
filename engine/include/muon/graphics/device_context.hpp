#pragma once

#include "muon/core/window.hpp"
#include "muon/graphics/queue.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <unordered_set>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::gfx {

    struct DeviceContextSpecification {
        const Window *window;
    };

    class DeviceContext : NoCopy, NoMove {
    public:
        DeviceContext(const DeviceContextSpecification &spec);
        ~DeviceContext();

    public:
        [[nodiscard]] VkInstance GetInstance() const;
        [[nodiscard]] VkSurfaceKHR GetSurface() const;
        [[nodiscard]] VkPhysicalDevice GetPhysicalDevice() const;
        [[nodiscard]] VkDevice GetDevice() const;

        [[nodiscard]] Queue &GetGraphicsQueue() const;
        [[nodiscard]] Queue &GetComputeQueue() const;
        [[nodiscard]] Queue &GetTransferQueue() const;

        [[nodiscard]] VmaAllocator GetAllocator() const;

    private:
        void CreateInstance(const Window &window);
        void CreateDebugMessenger();
        void CreateSurface(const Window &window);
        void SelectPhysicalDevice();
        void CreateLogicalDevice();
        void CreateAllocator();

    private:
        VkInstance m_instance = nullptr;

        #ifdef MU_DEBUG_ENABLED
        VkDebugUtilsMessengerEXT m_debugMessenger = nullptr;
        #endif

        VkSurfaceKHR m_surface = nullptr;

        VkPhysicalDevice m_physicalDevice = nullptr;
        std::unordered_set<const char *> m_enabledExtensions{};

        VkDevice m_device = nullptr;

        std::unique_ptr<Queue> m_graphicsQueue = nullptr;
        std::unique_ptr<Queue> m_computeQueue = nullptr;
        std::unique_ptr<Queue> m_transferQueue = nullptr;

        VmaAllocator m_allocator = nullptr;
    };

}
