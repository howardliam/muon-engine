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
    Mesh(
        const Context &context,
        vk::raii::CommandBuffer &commandBuffer,
        std::deque<Buffer> &uploadBuffers,
        const std::vector<uint8_t> &vertexData,
        uint32_t vertexStride,
        const std::vector<uint32_t> &indices
    );
    ~Mesh();

    void bind(vk::raii::CommandBuffer &commandBuffer);
    void draw(vk::raii::CommandBuffer &commandBuffer);

private:
    void createBuffer(
        vk::raii::CommandBuffer &commandBuffer,
        std::deque<Buffer> &uploadBuffers,
        const void *data,
        vk::DeviceSize instanceSize,
        size_t instanceCount,
        vk::BufferUsageFlagBits bufferUsage,
        std::unique_ptr<Buffer> &buffer
    );

private:
    const Context &m_context;

    std::unique_ptr<Buffer> m_vertexBuffer{nullptr};
    uint32_t m_vertexCount;

    std::unique_ptr<Buffer> m_indexBuffer{nullptr};
    uint32_t m_indexCount;
};

} // namespace muon::graphics
