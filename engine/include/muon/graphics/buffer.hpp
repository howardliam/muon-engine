#pragma once

#include "muon/graphics/device_context.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    class Buffer {
    public:
        struct Spec {
            const DeviceContext *device{nullptr};
            VkDeviceSize instanceSize{};
            uint32_t instanceCount{};
            VkBufferUsageFlags usageFlags{};
            VkMemoryPropertyFlags memoryPropertyFlags{};
            VmaMemoryUsage memoryUsage{VMA_MEMORY_USAGE_AUTO};
            VmaAllocationCreateFlags allocationFlags{};
            VkDeviceSize minOffsetAlignment{1};
        };

    public:
        Buffer(const Spec &spec);
        ~Buffer();

        [[nodiscard]] auto Map() -> VkResult;
        auto Unmap() -> void;

        auto Write(const void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;
        auto Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;
        auto Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;

    public:
        [[nodiscard]] auto Get() const -> VkBuffer;
        [[nodiscard]] auto GetSize() const -> VkDeviceSize;
        [[nodiscard]] auto GetMappedMemory() const -> void *;
        [[nodiscard]] auto GetDeviceAddress() const -> VkDeviceAddress;

        [[nodiscard]] auto GetInstanceCount() const -> uint32_t;
        [[nodiscard]] auto GetInstanceSize() const -> VkDeviceSize;

        [[nodiscard]] auto GetDescriptorInfo() const -> const VkDescriptorBufferInfo &;

    private:
        const DeviceContext &m_device;

        VkBuffer m_buffer{nullptr};
        VmaAllocation m_allocation{nullptr};
        VkDeviceSize m_size{};
        void *m_mapped{nullptr};
        VkDeviceAddress m_deviceAddress{};

        uint32_t m_instanceCount{};
        VkDeviceSize m_instanceSize{};
        VkDeviceSize m_alignmentSize{};
        VkBufferUsageFlags m_usageFlags{};

        VkDescriptorBufferInfo m_descriptorInfo{};
    };

}
