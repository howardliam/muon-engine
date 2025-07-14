#include "muon/graphics/mesh.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"

#include <memory>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

Mesh::Mesh(const Spec &spec) : m_device(*spec.device) {
    MU_CORE_ASSERT(spec.cmd != nullptr, "there must be a valid command buffer");
    MU_CORE_ASSERT(spec.uploadBuffers != nullptr, "there must be a valid upload buffer vector");

    CreateVertexBuffer(spec.cmd, spec.uploadBuffers, *spec.vertexData, spec.vertexStride);
    CreateIndexBuffer(spec.cmd, spec.uploadBuffers, *spec.indices);

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

auto Mesh::CreateVertexBuffer(
    VkCommandBuffer cmd, std::deque<Buffer> *uploadBuffers, const std::vector<uint8_t> &data, uint32_t stride
) -> void {
    auto vertexSize = stride;
    m_vertexCount = data.size() / vertexSize;

    Buffer::Spec stagingSpec{};
    stagingSpec.device = &m_device;
    stagingSpec.instanceSize = vertexSize;
    stagingSpec.instanceCount = m_vertexCount;
    stagingSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    Buffer &stagingBuffer = uploadBuffers->emplace_back(stagingSpec);

    auto result = stagingBuffer.Map();
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to map staging buffer");

    stagingBuffer.Write(data.data());

    Buffer::Spec spec{};
    spec.device = &m_device;
    spec.instanceSize = vertexSize;
    spec.instanceCount = m_vertexCount;
    spec.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    m_vertexBuffer = std::make_unique<Buffer>(spec);

    VkBufferCopy copy{};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stagingBuffer.GetSize();

    vkCmdCopyBuffer(cmd, stagingBuffer.Get(), m_vertexBuffer->Get(), 1, &copy);
}

auto Mesh::CreateIndexBuffer(VkCommandBuffer cmd, std::deque<Buffer> *uploadBuffers, const std::vector<uint32_t> &data) -> void {
    m_indexCount = data.size();

    Buffer::Spec stagingSpec{};
    stagingSpec.device = &m_device;
    stagingSpec.instanceSize = sizeof(uint32_t);
    stagingSpec.instanceCount = m_indexCount;
    stagingSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    Buffer &stagingBuffer = uploadBuffers->emplace_back(stagingSpec);

    auto result = stagingBuffer.Map();
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to map staging buffer");

    stagingBuffer.Write(data.data());

    Buffer::Spec spec{};
    spec.device = &m_device;
    spec.instanceSize = sizeof(uint32_t);
    spec.instanceCount = m_indexCount;
    spec.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    m_indexBuffer = std::make_unique<Buffer>(spec);

    VkBufferCopy copy{};
    copy.srcOffset = 0;
    copy.dstOffset = 0;
    copy.size = stagingBuffer.GetSize();

    vkCmdCopyBuffer(cmd, stagingBuffer.Get(), m_indexBuffer->Get(), 1, &copy);
}

} // namespace muon::graphics
