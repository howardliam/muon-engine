#include "muon/graphics/mesh.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <memory>

namespace muon::graphics {

Mesh::Mesh(const Spec &spec) : m_context(*spec.context) {
    core::expect(spec.commandBuffer != nullptr, "there must be a valid command buffer");
    core::expect(spec.uploadBuffers != nullptr, "there must be a valid upload buffer vector");

    createBuffer(
        *spec.commandBuffer, spec.uploadBuffers, spec.vertexData->data(), spec.vertexStride,
        spec.vertexData->size() / spec.vertexStride, vk::BufferUsageFlagBits::eVertexBuffer, m_vertexBuffer
    );

    createBuffer(
        *spec.commandBuffer, spec.uploadBuffers, spec.indices->data(), sizeof(uint32_t), spec.indices->size(),
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
    Buffer::Spec stagingSpec{};
    stagingSpec.context = &m_context;
    stagingSpec.instanceSize = instanceSize;
    stagingSpec.instanceCount = instanceCount;
    stagingSpec.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    Buffer &stagingBuffer = uploadBuffers->emplace_back(stagingSpec);

    auto result = stagingBuffer.map();
    core::expect(!result, "failed to map staging buffer");

    stagingBuffer.write(data);

    Buffer::Spec spec{};
    spec.context = &m_context;
    spec.instanceSize = instanceSize;
    spec.instanceCount = instanceCount;
    spec.usageFlags = bufferUsage | vk::BufferUsageFlagBits::eTransferDst;
    buffer = std::make_unique<Buffer>(spec);

    vk::BufferCopy copy;
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stagingBuffer.getSize();

    commandBuffer.copyBuffer(stagingBuffer.get(), buffer->get(), copy);
}

} // namespace muon::graphics
