#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/queue.hpp"

#include <string>
#include <unordered_set>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Context : NoCopy, NoMove {
public:
    struct Spec {
        const Window *window{nullptr};
    };

public:
    Context(const Spec &spec);
    ~Context();

    [[nodiscard]] auto DeviceWait() -> VkResult;

public:
    auto GetInstance() const -> VkInstance;
    auto GetSurface() const -> VkSurfaceKHR;
    auto GetPhysicalDevice() const -> VkPhysicalDevice;
    auto GetDevice() const -> VkDevice;

    auto GetGraphicsQueue() const -> Queue &;
    auto GetComputeQueue() const -> Queue &;
    auto GetTransferQueue() const -> Queue &;

    auto GetAllocator() const -> VmaAllocator;

private:
    auto CreateInstance(const Window &window) -> void;
#ifdef MU_DEBUG_ENABLED
    auto CreateDebugMessenger() -> void;
#endif
    auto CreateSurface(const Window &window) -> void;
    auto SelectPhysicalDevice() -> void;
    auto CreateLogicalDevice() -> void;
    auto CreateAllocator() -> void;

private:
    VkInstance m_instance{nullptr};

#ifdef MU_DEBUG_ENABLED
    VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
#endif

    VkSurfaceKHR m_surface{nullptr};

    VkPhysicalDevice m_physicalDevice{nullptr};
    std::unordered_set<std::string> m_enabledExtensions{};

    VkDevice m_device{nullptr};

    std::unique_ptr<Queue> m_graphicsQueue{nullptr};
    std::unique_ptr<Queue> m_computeQueue{nullptr};
    std::unique_ptr<Queue> m_transferQueue{nullptr};

    VmaAllocator m_allocator{nullptr};
};

} // namespace muon::graphics
