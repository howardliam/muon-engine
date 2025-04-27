#pragma once

#include "muon/engine/buffer.hpp"
#include "muon/engine/device.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class Model {
    public:
        Model(Device &device, const std::vector<uint8_t> &vertices, uint32_t stride, const std::vector<uint32_t> &indices);
        ~Model();

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

        bool hasIndexBuffer{false};
        std::unique_ptr<Buffer> indexBuffer{nullptr};
        uint32_t indexCount{0};

        void createVertexBuffer(const std::vector<uint8_t> &vertices, uint32_t stride);
        void createIndexBuffer(const std::vector<uint32_t> &indices);
    };

    template<typename T>
    std::vector<uint8_t> Model::getRawVertexData(const std::vector<T> &vertices) {
        const uint8_t *data = reinterpret_cast<const uint8_t *>(vertices.data());
        return std::vector<uint8_t>(data, data + vertices.size() * sizeof(T));
    }
}
