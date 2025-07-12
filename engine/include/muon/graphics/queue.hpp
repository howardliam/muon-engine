#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"

#include <limits>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Queue : NoCopy, NoMove {
public:
    struct Spec {
        VkDevice device{nullptr};
        uint32_t queueFamilyIndex{std::numeric_limits<uint32_t>().max()};
        uint32_t queueIndex{std::numeric_limits<uint32_t>().max()};
        const char *name{nullptr};
    };

public:
    Queue(const Spec &spec);
    ~Queue();

public:
    [[nodiscard]] auto BeginCommands() -> VkCommandBuffer;
    auto EndCommands(VkCommandBuffer cmd) -> void;

public:
    [[nodiscard]] auto GetFamilyIndex() const -> uint32_t;
    [[nodiscard]] auto GetIndex() const -> uint32_t;

    [[nodiscard]] auto Get() const -> VkQueue;
    [[nodiscard]] auto GetCommandPool() const -> VkCommandPool;

private:
    VkDevice m_device{nullptr};
    uint32_t m_familyIndex{std::numeric_limits<uint32_t>().max()};
    uint32_t m_index{std::numeric_limits<uint32_t>().max()};
    const char *m_name{nullptr};

    VkQueue m_queue{nullptr};
    VkCommandPool m_commandPool{nullptr};
};

} // namespace muon::graphics
