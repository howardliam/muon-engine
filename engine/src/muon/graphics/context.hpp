#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/queue.hpp"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <string>
#include <unordered_set>

namespace muon::graphics {

class Context : NoCopy, NoMove {
public:
    struct Spec {
        const Window *window{nullptr};
        bool debug{false};
    };

public:
    Context(const Spec &spec);
    ~Context();

public:
    auto getInstance() -> vk::raii::Instance &;
    auto getInstance() const -> const vk::raii::Instance &;

    auto getSurface() -> vk::raii::SurfaceKHR &;
    auto getSurface() const -> const vk::raii::SurfaceKHR &;

    auto getPhysicalDevice() -> vk::raii::PhysicalDevice &;
    auto getPhysicalDevice() const -> const vk::raii::PhysicalDevice &;

    auto getDevice() -> vk::raii::Device &;
    auto getDevice() const -> const vk::raii::Device &;

    auto getGraphicsQueue() -> Queue &;
    auto getGraphicsQueue() const -> const Queue &;

    auto getComputeQueue() -> Queue &;
    auto getComputeQueue() const -> const Queue &;

    auto getTransferQueue() -> Queue &;
    auto getTransferQueue() const -> const Queue &;

    auto getAllocator() -> vma::Allocator &;
    auto getAllocator() const -> const vma::Allocator &;

private:
    auto createInstance(const Window &window, bool debug) -> void;
    auto createDebugMessenger(bool debug) -> void;
    auto createSurface(const Window &window) -> void;
    auto selectPhysicalDevice() -> void;
    auto createLogicalDevice() -> void;
    auto createAllocator() -> void;

private:
    vk::raii::Context m_context;
    vk::raii::Instance m_instance{nullptr};

    vk::raii::DebugUtilsMessengerEXT m_debugMessenger{nullptr};

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
