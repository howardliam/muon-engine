#include "muon/graphics/mesh.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <memory>

namespace muon::graphics {

Mesh::Mesh(const Spec &spec) : m_context(*spec.context) {
    MU_CORE_ASSERT(spec.commandBuffer != nullptr, "there must be a valid command buffer");
    MU_CORE_ASSERT(spec.uploadBuffers != nullptr, "there must be a valid upload buffer vector");

    CreateBuffer(
        *spec.commandBuffer, spec.uploadBuffers, spec.vertexData->data(), spec.vertexStride,
        spec.vertexData->size() / spec.vertexStride, vk::BufferUsageFlagBits::eVertexBuffer, m_vertexBuffer
    );

    CreateBuffer(
        *spec.commandBuffer, spec.uploadBuffers, spec.indices->data(), sizeof(uint32_t), spec.indices->size(),
        vk::BufferUsageFlagBits::eIndexBuffer, m_indexBuffer
    );

    MU_CORE_DEBUG("created mesh with: {} vertices", m_vertexCount);
}

Mesh::~Mesh() { MU_CORE_DEBUG("destroyed mesh"); }

auto Mesh::Bind(vk::raii::CommandBuffer &commandBuffer) -> void {
    const auto &buffer = m_vertexBuffer->Get();
    const VkDeviceSize offset = 0;

    commandBuffer.bindVertexBuffers(0, *buffer, offset);
    commandBuffer.bindIndexBuffer(m_indexBuffer->Get(), 0, vk::IndexType::eUint32);
}

auto Mesh::Draw(vk::raii::CommandBuffer &commandBuffer) -> void { commandBuffer.drawIndexed(m_indexCount, 1, 0, 0, 0); }

auto Mesh::CreateBuffer(
    vk::raii::CommandBuffer &commandBuffer, std::deque<Buffer> *uploadBuffers, const void *data, VkDeviceSize instanceSize,
    size_t instanceCount, vk::BufferUsageFlagBits bufferUsage, std::unique_ptr<Buffer> &buffer
) -> void {
    Buffer::Spec stagingSpec{};
    stagingSpec.context = &m_context;
    stagingSpec.instanceSize = instanceSize;
    stagingSpec.instanceCount = instanceCount;
    stagingSpec.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    Buffer &stagingBuffer = uploadBuffers->emplace_back(stagingSpec);

    auto result = stagingBuffer.Map();
    MU_CORE_ASSERT(!result, "failed to map staging buffer");

    stagingBuffer.Write(data);

    Buffer::Spec spec{};
    spec.context = &m_context;
    spec.instanceSize = instanceSize;
    spec.instanceCount = instanceCount;
    spec.usageFlags = bufferUsage | vk::BufferUsageFlagBits::eTransferDst;
    buffer = std::make_unique<Buffer>(spec);

    vk::BufferCopy copy;
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stagingBuffer.GetSize();

    commandBuffer.copyBuffer(stagingBuffer.Get(), buffer->Get(), copy);
}

} // namespace muon::graphics
