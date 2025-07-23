#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <functional>
#include <limits>

namespace muon::graphics {

class Queue : NoCopy, NoMove {
public:
    struct Spec {
        const vk::raii::Device *device{nullptr};
        uint32_t queueFamilyIndex{std::numeric_limits<uint32_t>().max()};
        uint32_t queueIndex{std::numeric_limits<uint32_t>().max()};
        const char *name{nullptr};
    };

public:
    Queue(const Spec &spec);
    ~Queue();

public:
    auto ExecuteCommands(std::function<void(vk::raii::CommandBuffer &commandBuffer)> const &recordFn);

public:
    auto GetFamilyIndex() const -> uint32_t;
    auto GetIndex() const -> uint32_t;

    auto Get() -> vk::raii::Queue &;
    auto Get() const -> const vk::raii::Queue &;

    auto GetCommandPool() -> vk::raii::CommandPool &;
    auto GetCommandPool() const -> const vk::raii::CommandPool &;

private:
    const vk::raii::Device &m_device;
    uint32_t m_familyIndex{std::numeric_limits<uint32_t>().max()};
    uint32_t m_index{std::numeric_limits<uint32_t>().max()};
    const char *m_name{nullptr};

    vk::raii::Queue m_queue{nullptr};
    vk::raii::CommandPool m_commandPool{nullptr};
};

} // namespace muon::graphics
