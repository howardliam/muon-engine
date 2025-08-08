#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <functional>
#include <string_view>

namespace muon::graphics {

class Queue : NoCopy, NoMove {
public:
    Queue(
        const vk::raii::Device &device,
        uint32_t familyIndex,
        uint32_t index,
        std::string_view name
    );
    ~Queue();

public:
    void executeCommands(std::function<void(vk::raii::CommandBuffer &commandBuffer)> const &recordFn);

public:
    auto get() -> vk::raii::Queue &;
    auto get() const -> const vk::raii::Queue &;

    auto getCommandPool() -> vk::raii::CommandPool &;
    auto getCommandPool() const -> const vk::raii::CommandPool &;

    auto getFamilyIndex() const -> uint32_t;
    auto getIndex() const -> uint32_t;

private:
    const vk::raii::Device &m_device;

    vk::raii::Queue m_queue{nullptr};
    vk::raii::CommandPool m_commandPool{nullptr};

    uint32_t m_familyIndex;
    uint32_t m_index;

    const std::string m_name;
};

} // namespace muon::graphics
