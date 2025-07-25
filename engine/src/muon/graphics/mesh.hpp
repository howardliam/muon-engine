#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <deque>
#include <memory>

namespace muon::graphics {

class Mesh : NoCopy, NoMove {
public:
    struct Spec {
        const Context *context{nullptr};
        vk::raii::CommandBuffer *commandBuffer{nullptr};
        std::deque<Buffer> *uploadBuffers{nullptr};

        const std::vector<uint8_t> *vertexData{nullptr};
        uint32_t vertexStride{0};
        const std::vector<uint32_t> *indices{nullptr};
    };

public:
    Mesh(const Spec &spec);
    ~Mesh();

    auto bind(vk::raii::CommandBuffer &commandBuffer) -> void;
    auto draw(vk::raii::CommandBuffer &commandBuffer) -> void;

private:
    auto createBuffer(
        vk::raii::CommandBuffer &commandBuffer, std::deque<Buffer> *uploadBuffers, const void *data, vk::DeviceSize instanceSize,
        size_t instanceCount, vk::BufferUsageFlagBits bufferUsage, std::unique_ptr<Buffer> &buffer
    ) -> void;

private:
    const Context &m_context;

    std::unique_ptr<Buffer> m_vertexBuffer{nullptr};
    uint32_t m_vertexCount;

    std::unique_ptr<Buffer> m_indexBuffer{nullptr};
    uint32_t m_indexCount;
};

} // namespace muon::graphics
