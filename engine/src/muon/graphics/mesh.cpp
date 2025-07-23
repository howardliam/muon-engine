#include "muon/graphics/mesh.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"

#include <memory>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

Mesh::Mesh(const Spec &spec) : m_context(*spec.context) {
    MU_CORE_ASSERT(spec.cmd != nullptr, "there must be a valid command buffer");
    MU_CORE_ASSERT(spec.uploadBuffers != nullptr, "there must be a valid upload buffer vector");

    CreateBuffer(
        spec.cmd, spec.uploadBuffers, spec.vertexData->data(), spec.vertexStride, spec.vertexData->size() / spec.vertexStride,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_vertexBuffer
    );

    CreateBuffer(
        spec.cmd, spec.uploadBuffers, spec.indices->data(), sizeof(uint32_t), spec.indices->size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_indexBuffer
    );

    MU_CORE_DEBUG("created mesh with: {} vertices", m_vertexCount);
}

Mesh::~Mesh() { MU_CORE_DEBUG("destroyed mesh"); }

auto Mesh::Bind(VkCommandBuffer cmd) -> void {
    const auto buffer = m_vertexBuffer->Get();
    const VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
    vkCmdBindIndexBuffer(cmd, m_indexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);
}

auto Mesh::Draw(VkCommandBuffer cmd) -> void { vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, 0); }

auto Mesh::CreateBuffer(
    VkCommandBuffer cmd, std::deque<Buffer> *uploadBuffers, const void *data, VkDeviceSize instanceSize, size_t instanceCount,
    VkBufferUsageFlagBits bufferUsage, std::unique_ptr<Buffer> &buffer
) -> void {
    Buffer::Spec stagingSpec{};
    stagingSpec.context = &m_context;
    stagingSpec.instanceSize = instanceSize;
    stagingSpec.instanceCount = instanceCount;
    stagingSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    Buffer &stagingBuffer = uploadBuffers->emplace_back(stagingSpec);

    auto result = stagingBuffer.Map();
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to map staging buffer");

    stagingBuffer.Write(data);

    Buffer::Spec spec{};
    spec.context = &m_context;
    spec.instanceSize = instanceSize;
    spec.instanceCount = instanceCount;
    spec.usageFlags = bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer = std::make_unique<Buffer>(spec);

    VkBufferCopy copy{};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stagingBuffer.GetSize();

    vkCmdCopyBuffer(cmd, stagingBuffer.Get(), buffer->Get(), 1, &copy);
}

} // namespace muon::graphics
