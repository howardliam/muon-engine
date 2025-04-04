#pragma once

#include "muon/engine/device.hpp"
#include <vk_mem_alloc_handles.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>

namespace muon::engine {

    class Buffer {
    public:
        Buffer(
            Device &device,
            vk::DeviceSize instanceSize,
            uint32_t instanceCount,
            vk::BufferUsageFlags usageFlags,
            vk::DeviceSize minOffsetAlignment = 1
        );
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        [[nodiscard]] vk::Result map(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void *data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        [[nodiscard]] vk::Result flush(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        [[nodiscard]] vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        [[nodiscard]] vk::Result invalidate(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);

        void writeToIndex(void *data, int32_t index);
        [[nodiscard]] vk::Result flushIndex(int32_t index);
        [[nodiscard]] vk::DescriptorBufferInfo descriptorInfoForIndex(int32_t index);
        [[nodiscard]] vk::Result invalidateIndex(int32_t index);

        [[nodiscard]] vk::Buffer getBuffer() const;
        [[nodiscard]] vk::DeviceSize getBufferSize() const;
        void *getMappedMemory() const;
        [[nodiscard]] uint32_t getInstanceCount() const;
        [[nodiscard]] vk::DeviceSize getInstanceSize() const;
        [[nodiscard]] vk::DeviceSize getAlignmentSize() const;
        [[nodiscard]] vk::BufferUsageFlags getUsageFlags() const;

    private:
        Device &device;

        vk::Buffer buffer;
        vk::DeviceSize bufferSize;
        vma::Allocation allocation;
        vk::DeviceMemory memory = nullptr;
        void *mapped = nullptr;

        uint32_t instanceCount;
        vk::DeviceSize instanceSize;
        vk::DeviceSize alignmentSize;
        vk::BufferUsageFlags usageFlags;
    };

}
