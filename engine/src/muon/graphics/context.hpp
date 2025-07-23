#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/queue.hpp"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <expected>
#include <string>
#include <unordered_set>

namespace muon::graphics {

class Context : NoCopy, NoMove {
public:
    struct Spec {
        const Window *window{nullptr};
    };

public:
    Context(const Spec &spec);
    ~Context();

    auto DeviceWaitIdle() -> std::expected<void, vk::Result>;

public:
    auto GetInstance() -> vk::raii::Instance &;
    auto GetInstance() const -> const vk::raii::Instance &;

    auto GetSurface() -> vk::raii::SurfaceKHR &;
    auto GetSurface() const -> const vk::raii::SurfaceKHR &;

    auto GetPhysicalDevice() -> vk::raii::PhysicalDevice &;
    auto GetPhysicalDevice() const -> const vk::raii::PhysicalDevice &;

    auto GetDevice() -> vk::raii::Device &;
    auto GetDevice() const -> const vk::raii::Device &;

    auto GetGraphicsQueue() const -> Queue &;
    auto GetComputeQueue() const -> Queue &;
    auto GetTransferQueue() const -> Queue &;

    auto GetAllocator() const -> vma::Allocator;

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
    vk::raii::Context m_context;
    vk::raii::Instance m_instance{nullptr};

#ifdef MU_DEBUG_ENABLED
    vk::raii::DebugUtilsMessengerEXT m_debugMessenger{nullptr};
#endif

    vk::raii::SurfaceKHR m_surface{nullptr};

    vk::raii::PhysicalDevice m_physicalDevice{nullptr};
    std::unordered_set<std::string> m_enabledExtensions{};

    vk::raii::Device m_device{nullptr};

    std::unique_ptr<Queue> m_graphicsQueue{nullptr};
    std::unique_ptr<Queue> m_computeQueue{nullptr};
    std::unique_ptr<Queue> m_transferQueue{nullptr};

    vma::Allocator m_allocator{nullptr};
};

} // namespace muon::graphics
