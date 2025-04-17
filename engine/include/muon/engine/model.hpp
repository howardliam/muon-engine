#pragma once

#include "muon/engine/buffer.hpp"
#include "muon/engine/device.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    template<typename T>
    class Model {
    public:
        Model(Device &device, const std::vector<T> &vertices, const std::vector<uint32_t> &indices);
        ~Model() = default;

        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        void bind(vk::CommandBuffer commandBuffer) const;
        void draw(vk::CommandBuffer commandBuffer) const;

    private:
        Device &device;

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount{0};

        bool hasIndexBuffer{false};
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount{0};

        void createVertexBuffer(const std::vector<T> &vertices);
        void createIndexBuffer(const std::vector<uint32_t> &indices);
    };

    template<typename T>
    Model<T>::Model(
        Device &device,
        const std::vector<T> &vertices,
        const std::vector<uint32_t> &indices
    ) : device(device) {
        createVertexBuffer(vertices);
        createIndexBuffer(indices);
    }

    template<typename T>
    void Model<T>::createVertexBuffer(const std::vector<T> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());

        size_t vertexSize = sizeof(T);
        vk::DeviceSize bufferSize = vertexSize * vertexCount;

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

        device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    template<typename T>
    void Model<T>::createIndexBuffer(const std::vector<uint32_t> &indices) {
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

    template<typename T>
    void Model<T>::bind(vk::CommandBuffer commandBuffer) const {
        const vk::Buffer buffer = vertexBuffer->getBuffer();
        const vk::DeviceSize offset = 0;

        commandBuffer.bindVertexBuffers(0, 1, &buffer, &offset);

        if (hasIndexBuffer) {
            commandBuffer.bindIndexBuffer(indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
        }
    }

    template<typename T>
    void Model<T>::draw(vk::CommandBuffer commandBuffer) const {
        if (hasIndexBuffer) {
            commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
        } else {
            commandBuffer.draw(vertexCount, 1, 0, 0);
        }
    }
}
