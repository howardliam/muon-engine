#include "muon/graphics/mesh.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include "vulkan/vulkan_enums.hpp"

namespace muon::graphics {

Mesh::Mesh(
    const Context &context,
    vk::raii::CommandBuffer &commandBuffer,
    std::deque<Buffer> &uploadBuffers,
    const std::vector<uint8_t> &vertexData,
    uint32_t vertexStride,
    const std::vector<uint32_t> &indices
) : m_context{context} {
    createBuffer(
        commandBuffer,
        uploadBuffers,
        vertexData.data(),
        vertexStride,
        vertexData.size() / vertexStride,
        vk::BufferUsageFlagBits::eVertexBuffer,
        m_vertexBuffer
    );

    createBuffer(
        commandBuffer,
        uploadBuffers,
        indices.data(),
        sizeof(uint32_t),
        indices.size(),
        vk::BufferUsageFlagBits::eIndexBuffer,
        m_indexBuffer
    );

    core::debug("created mesh with: {} vertices", m_vertexCount);
}

Mesh::~Mesh() { core::debug("destroyed mesh"); }

void Mesh::bind(vk::raii::CommandBuffer &commandBuffer) {
    const auto &buffer = m_vertexBuffer->get();
    const VkDeviceSize offset = 0;

    commandBuffer.bindVertexBuffers(0, *buffer, offset);
    commandBuffer.bindIndexBuffer(m_indexBuffer->get(), 0, vk::IndexType::eUint32);
}

void Mesh::draw(vk::raii::CommandBuffer &commandBuffer) { commandBuffer.drawIndexed(m_indexCount, 1, 0, 0, 0); }

void Mesh::createBuffer(
    vk::raii::CommandBuffer &commandBuffer,
    std::deque<Buffer> &uploadBuffers,
    const void *data,
    vk::DeviceSize instanceSize,
    size_t instanceCount,
    vk::BufferUsageFlagBits bufferUsage,
    std::unique_ptr<Buffer> &buffer
) {
    Buffer &stagingBuffer = uploadBuffers.emplace_back(
        m_context,
        instanceSize,
        instanceCount,
        vk::BufferUsageFlagBits::eTransferSrc
    );

    auto result = stagingBuffer.map();
    core::expect(!result, "failed to map staging buffer");

    stagingBuffer.write(data);

    buffer = std::make_unique<Buffer>(
        m_context,
        instanceSize,
        instanceCount,
        bufferUsage | vk::BufferUsageFlagBits::eTransferDst
    );

    vk::BufferCopy copy;
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stagingBuffer.getSize();

    commandBuffer.copyBuffer(stagingBuffer.get(), buffer->get(), copy);
}

} // namespace muon::graphics
