#pragma once

#include "muon/engine/buffer.hpp"
#include "muon/engine/device.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class Model {
    public:
        Model(Device &device);
        ~Model();

        Model(const Model &) = delete;
        Model &operator=(const Model &) = delete;

        void bind(vk::CommandBuffer commandBuffer);
        void draw(vk::CommandBuffer commandBuffer);

    private:
        Device &device;

        std::unique_ptr<Buffer> vertexBuffer;
        uint32_t vertexCount{0};

        bool hasIndexBuffer{false};
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount{0};

        void createVertexBuffer(const std::vector<uint8_t> &vertices);
        void createIndexBuffer(const std::vector<uint32_t> &indices);
    };

}
