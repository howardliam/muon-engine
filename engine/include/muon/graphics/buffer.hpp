#pragma once

#include "muon/graphics/device_context.hpp"
#include <vulkan/vulkan_core.h>

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

        auto Write(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;
        auto Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;
        auto Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;

    public:
        [[nodiscard]] auto Get() const -> VkBuffer;
        [[nodiscard]] auto GetSize() const -> VkDeviceSize;
        [[nodiscard]] auto GetMappedMemory() const -> void *;

        [[nodiscard]] auto GetDescriptorInfo() const -> const VkDescriptorBufferInfo &;

    private:
        const DeviceContext &m_device;

        VkBuffer m_buffer{nullptr};
        VkDeviceSize m_size{};
        VmaAllocation m_allocation{nullptr};
        void *m_mapped{nullptr};

        uint32_t m_instanceCount{};
        VkDeviceSize m_instanceSize{};
        VkDeviceSize m_alignmentSize{};
        VkBufferUsageFlags m_usageFlags{};

        VkDescriptorBufferInfo m_descriptorInfo{};
    };

}
