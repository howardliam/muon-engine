#include "muon/engine/buffer.hpp"

#include <cstring>
#include <vk_mem_alloc_enums.hpp>
#include <vk_mem_alloc_structs.hpp>
#include <print>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    Buffer::Buffer(
        Device &device,
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        vk::BufferUsageFlags usageFlags,
        vma::MemoryUsage memoryUsage,
        vk::DeviceSize minOffsetAlignment
    ) : device(device), instanceSize(instanceSize), instanceCount(instanceCount), usageFlags(usageFlags) {
        auto getAlignment = [](vk::DeviceSize instanceSize, vk::DeviceSize minOffsetAlignment) -> vk::DeviceSize {
            if (minOffsetAlignment > 0) {
                return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
            }
            return instanceSize;
        };

        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;

        device.createBuffer(bufferSize, usageFlags, memoryUsage, buffer, allocation);
    }

    Buffer::~Buffer() {
        unmap();
        device.getAllocator().destroyBuffer(buffer, allocation);
    }

    vk::Result Buffer::map() {
        return device.getAllocator().mapMemory(allocation, &mapped);
    }

    void Buffer::unmap() {
        if (mapped == nullptr) {
            return;
        }

        device.getAllocator().unmapMemory(allocation);
        mapped = nullptr;
    }

    void Buffer::writeToBuffer(void *data, vk::DeviceSize size, vk::DeviceSize offset) {
        if (size == vk::WholeSize) {
            memcpy(mapped, data, bufferSize);
        } else {
            auto memoryOffset = static_cast<char *>(mapped);
            memoryOffset += offset;
            memcpy(memoryOffset, data, size);
        }
    }

    void Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
        device.getAllocator().flushAllocation(allocation, offset, size);
    }

    vk::DescriptorBufferInfo Buffer::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
        return vk::DescriptorBufferInfo{ buffer, offset, size };
    }

    void Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
        device.getAllocator().invalidateAllocation(allocation, offset, size);
    }

}
