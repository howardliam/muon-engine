#include "muon/engine/mesh.hpp"

#include "muon/engine/buffer.hpp"
#include "muon/engine/device.hpp"
#include "muon/log/logger.hpp"

namespace muon::engine {

    Mesh::Mesh(
        Device &device,
        const std::vector<uint8_t> &vertices,
        uint32_t stride,
        const std::vector<uint32_t> &indices
    ) : device(device) {
        createVertexBuffer(vertices, stride);
        createIndexBuffer(indices);
        log::globalLogger->debug("created model with: {} vertices, {} faces", vertexCount, indices.size() / 3);
    }

    Mesh::~Mesh() {
        log::globalLogger->debug("destroyed model");
    }

    void Mesh::bind(vk::CommandBuffer commandBuffer) const {
        const vk::Buffer buffer = vertexBuffer->getBuffer();
        const vk::DeviceSize offset = 0;

        commandBuffer.bindVertexBuffers(0, 1, &buffer, &offset);
        commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
    }

    void Mesh::draw(vk::CommandBuffer commandBuffer) const {
        commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
    }

    void Mesh::createVertexBuffer(const std::vector<uint8_t> &vertices, uint32_t stride) {
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

    void Mesh::createIndexBuffer(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());

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
