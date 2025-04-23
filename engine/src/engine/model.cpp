#include "muon/engine/model.hpp"

namespace muon::engine {

    Model::Model(
        Device &device,
        const std::vector<uint8_t> &vertices,
        uint32_t stride,
        const std::vector<uint32_t> &indices
    ) : device(device) {
        createVertexBuffer(vertices, stride);
        createIndexBuffer(indices);
    }

    void Model::bind(vk::CommandBuffer commandBuffer) const {
        const vk::Buffer buffer = vertexBuffer->getBuffer();
        const vk::DeviceSize offset = 0;

        commandBuffer.bindVertexBuffers(0, 1, &buffer, &offset);

        if (hasIndexBuffer) {
            commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
        }
    }

    void Model::draw(vk::CommandBuffer commandBuffer) const {
        if (hasIndexBuffer) {
            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
        } else {
            commandBuffer.draw(vertexCount, 1, 0, 0);
        }
    }

    void Model::createVertexBuffer(const std::vector<uint8_t> &vertices, uint32_t stride) {
        uint32_t vertexSize = stride;
        vertexCount = static_cast<uint32_t>(vertices.size() / vertexSize);

        Buffer stagingBuffer(
            device,
            vertexSize,
            vertexCount,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::MemoryUsage::eCpuOnly
        );

        if (stagingBuffer.map() != vk::Result::eSuccess) {
            throw std::runtime_error("failed to map staging buffer");
        }
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<Buffer>(
            device,
            vertexSize,
            vertexCount,
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vma::MemoryUsage::eGpuOnly
        );

        vk::DeviceSize bufferSize = vertices.size();

        device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void Model::createIndexBuffer(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }

        size_t indexSize = sizeof(uint32_t);
        vk::DeviceSize bufferSize = indexSize * indexCount;

        Buffer stagingBuffer(
            device,
            indexSize,
            indexCount,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::MemoryUsage::eCpuOnly
        );

        if (stagingBuffer.map() != vk::Result::eSuccess) {
            throw std::runtime_error("failed to map staging buffer");
        }
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<Buffer>(
            device,
            indexSize,
            indexCount,
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            vma::MemoryUsage::eGpuOnly
        );

        device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

}
