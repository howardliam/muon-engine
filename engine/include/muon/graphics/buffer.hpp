#pragma once

#include "muon/graphics/device_context.hpp"

namespace muon::graphics {

    struct BufferSpecification {
        const DeviceContext *device{nullptr};
        VkDeviceSize instanceSize{};
        uint32_t instanceCount{};
        VkBufferUsageFlags usageFlags{};
        VmaMemoryUsage memoryUsage{};
        VkDeviceSize minOffsetAlignment{1};
    };

    class Buffer {
    public:
        Buffer(const BufferSpecification &spec);
        ~Buffer();

        [[nodiscard]] auto Map() -> VkResult;
        auto Unmap() -> void;

    public:


    private:

    private:
        const DeviceContext &m_device;

        VkBuffer m_buffer{nullptr};
        VkDeviceSize m_size{};
        VmaAllocation m_allocation{nullptr};
        void *m_mapped{nullptr};

        uint32_t m_instanceCount;
        VkDeviceSize m_instanceSize;
        VkDeviceSize m_alignmentSize;
        VkBufferUsageFlags m_usageFlags;
    };

}
