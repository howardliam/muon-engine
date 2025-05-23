#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"

#include <memory>
#include <vector>
#include <cstdint>

#include <vulkan/vulkan.hpp>

namespace mu {

    class Device;
    class Buffer;

    class Mesh : NoCopy, NoMove {
    public:
        Mesh(Device &device, const std::vector<uint8_t> &vertices, uint32_t stride, const std::vector<uint32_t> &indices);
        ~Mesh();

        /**
         * @brief   binds the model to the command buffer.
         *
         * @param   commandBuffer   command buffer to bind to.
         */
        void bind(vk::CommandBuffer commandBuffer) const;

        /**
         * @brief   draws the model.
         *
         * @param   commandBuffer   command buffer to draw to.
         */
        void draw(vk::CommandBuffer commandBuffer) const;

        /**
         * @brief   converts the structured input into raw bytes.
         *
         * @param   vertices    structured vector of vertex data.
         *
         * @return  raw vector of bytes consisting of vertex data.
         */
        template<typename T>
        static std::vector<uint8_t> getRawVertexData(const std::vector<T> &vertices);

    private:
        Device &device;

        std::unique_ptr<Buffer> vertexBuffer{nullptr};
        uint32_t vertexCount{0};

        std::unique_ptr<Buffer> indexBuffer{nullptr};
        uint32_t indexCount{0};

        void createVertexBuffer(const std::vector<uint8_t> &vertices, uint32_t stride);
        void createIndexBuffer(const std::vector<uint32_t> &indices);
    };
}

#include "muon/engine/renderer/mesh.inl"
