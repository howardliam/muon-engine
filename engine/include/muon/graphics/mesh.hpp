#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/device_context.hpp"

#include <cstdint>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Mesh : NoCopy, NoMove {
public:
    struct Spec {
        const DeviceContext *device{nullptr};
        const std::vector<uint8_t> *vertexData{nullptr};
        uint32_t vertexStride{0};
        const std::vector<uint32_t> *indices{nullptr};
        VkCommandBuffer cmd{nullptr};
    };

public:
    Mesh(const Spec &spec);
    ~Mesh();

    auto Bind(VkCommandBuffer cmd) -> void;
    auto Draw(VkCommandBuffer cmd) -> void;

private:
    auto CreateVertexBuffer(VkCommandBuffer cmd, const std::vector<uint8_t> &data, uint32_t stride) -> void;
    auto CreateIndexBuffer(VkCommandBuffer cmd, const std::vector<uint32_t> &data) -> void;

private:
    const DeviceContext &m_device;

    std::unique_ptr<Buffer> m_vertexBuffer{nullptr};
    uint32_t m_vertexCount;

    std::unique_ptr<Buffer> m_indexBuffer{nullptr};
    uint32_t m_indexCount;
};

} // namespace muon::graphics
