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
    [[nodiscard]] auto GetInstance() const -> VkInstance;
    [[nodiscard]] auto GetSurface() const -> VkSurfaceKHR;
    [[nodiscard]] auto GetPhysicalDevice() const -> VkPhysicalDevice;
    [[nodiscard]] auto GetDevice() const -> VkDevice;

    [[nodiscard]] auto GetGraphicsQueue() const -> Queue &;
    [[nodiscard]] auto GetComputeQueue() const -> Queue &;
    [[nodiscard]] auto GetTransferQueue() const -> Queue &;

    [[nodiscard]] auto GetAllocator() const -> VmaAllocator;

private:
    auto CreateInstance(const Window &window) -> void;
    auto CreateDebugMessenger() -> void;
    auto CreateSurface(const Window &window) -> void;
    auto SelectPhysicalDevice() -> void;
    auto CreateLogicalDevice() -> void;
    auto CreateAllocator() -> void;

private:
    VkInstance m_instance{nullptr};

    VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};

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
