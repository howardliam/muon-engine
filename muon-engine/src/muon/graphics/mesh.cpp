#include "muon/graphics/mesh.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <memory>

namespace muon::graphics {

Mesh::Mesh(const Spec &spec) : m_context(spec.context) {
    createBuffer(
        spec.commandBuffer, spec.uploadBuffers, spec.vertexData->data(), spec.vertexStride,
        spec.vertexData->size() / spec.vertexStride, vk::BufferUsageFlagBits::eVertexBuffer, m_vertexBuffer
    );

    createBuffer(
        spec.commandBuffer, spec.uploadBuffers, spec.indices->data(), sizeof(uint32_t), spec.indices->size(),
        vk::BufferUsageFlagBits::eIndexBuffer, m_indexBuffer
    );

    core::debug("created mesh with: {} vertices", m_vertexCount);
}

Mesh::~Mesh() { core::debug("destroyed mesh"); }

auto Mesh::bind(vk::raii::CommandBuffer &commandBuffer) -> void {
    const auto &buffer = m_vertexBuffer->get();
    const VkDeviceSize offset = 0;

    commandBuffer.bindVertexBuffers(0, *buffer, offset);
    commandBuffer.bindIndexBuffer(m_indexBuffer->get(), 0, vk::IndexType::eUint32);
}

auto Mesh::draw(vk::raii::CommandBuffer &commandBuffer) -> void { commandBuffer.drawIndexed(m_indexCount, 1, 0, 0, 0); }

auto Mesh::createBuffer(
    vk::raii::CommandBuffer &commandBuffer, std::deque<Buffer> *uploadBuffers, const void *data, VkDeviceSize instanceSize,
    size_t instanceCount, vk::BufferUsageFlagBits bufferUsage, std::unique_ptr<Buffer> &buffer
) -> void {
    Buffer &stagingBuffer = uploadBuffers->emplace_back(
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
