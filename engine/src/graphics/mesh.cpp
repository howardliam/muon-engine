#include "muon/graphics/mesh.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::graphics {

    Mesh::Mesh(const Spec &spec) : m_device(*spec.device) {
        MU_CORE_ASSERT(spec.cmd != nullptr, "there must be a valid command buffer");

        CreateVertexBuffer(spec.cmd, *spec.vertexData, spec.vertexStride);
        CreateIndexBuffer(spec.cmd, *spec.indices);

        MU_CORE_DEBUG("created mesh with: {} vertices", m_vertexCount);
    }

    Mesh::~Mesh() {
        MU_CORE_DEBUG("destroyed mesh");
    }

    auto Mesh::Bind(VkCommandBuffer cmd) -> void {
        const auto buffer = m_vertexBuffer->Get();
        const VkDeviceSize offset = 0;

        vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
        vkCmdBindIndexBuffer(cmd, m_indexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);
    }

    auto Mesh::Draw(VkCommandBuffer cmd) -> void {
        vkCmdDrawIndexed(cmd, m_indexCount, 1, 0, 0, 0);
    }

    auto Mesh::CreateVertexBuffer(VkCommandBuffer cmd, const std::vector<uint8_t> &data, uint32_t stride) -> void {
        auto vertexSize = stride;
        m_vertexCount = data.size() / vertexSize;

        Buffer::Spec spec{};
        spec.device = &m_device;
        spec.instanceSize = vertexSize;
        spec.instanceCount = m_vertexCount;
        spec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        spec.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        Buffer stagingBuffer{spec};

        auto result = stagingBuffer.Map();
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to map staging buffer");

        stagingBuffer.Write(data.data());

        spec.usageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        spec.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        m_vertexBuffer = std::make_unique<Buffer>(spec);

        VkBufferCopy copy{};
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = stagingBuffer.GetSize();

        vkCmdCopyBuffer(cmd, stagingBuffer.Get(), m_vertexBuffer->Get(), 1, &copy);
    }

    auto Mesh::CreateIndexBuffer(VkCommandBuffer cmd, const std::vector<uint32_t> &data) -> void {
        m_indexCount = data.size();

        Buffer::Spec spec{};
        spec.device = &m_device;
        spec.instanceSize = sizeof(uint32_t);
        spec.instanceCount = m_indexCount;
        spec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        spec.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        Buffer stagingBuffer{spec};

        auto result = stagingBuffer.Map();
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to map staging buffer");

        stagingBuffer.Write(data.data());

        spec.usageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        spec.memoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        m_indexBuffer = std::make_unique<Buffer>(spec);

        VkBufferCopy copy{};
        copy.srcOffset = 0;
        copy.dstOffset = 0;
        copy.size = stagingBuffer.GetSize();

        vkCmdCopyBuffer(cmd, stagingBuffer.Get(), m_indexBuffer->Get(), 1, &copy);
    }

}
