#include "muon/engine/renderer/buffer.hpp"

#include <cstring>
#include "muon/engine/renderer/device.hpp"
#include "muon/engine/core/log.hpp"
#include "muon/engine/utils/pretty_print.hpp"

namespace muon {

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

        MU_CORE_DEBUG("created buffer with size: {}", pp::parseBytes(bufferSize));
    }

    Buffer::~Buffer() {
        unmap();
        device.getAllocator().destroyBuffer(buffer, allocation);
        MU_CORE_DEBUG("destroyed buffer");
    }

    vk::Result Buffer::map() {
        return device.getAllocator().mapMemory(allocation, &mapped);
    }

    void Buffer::unmap() {
        if (mapped == nullptr) { return; }

        device.getAllocator().unmapMemory(allocation);
        mapped = nullptr;
    }

    void Buffer::writeToBuffer(void *data, vk::DeviceSize size, vk::DeviceSize offset) {
        if (size == vk::WholeSize) {
            std::memcpy(mapped, data, bufferSize);
        } else {
            auto memoryOffset = static_cast<char *>(mapped);
            memoryOffset += offset;
            std::memcpy(memoryOffset, data, size);
        }
    }

    void Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
        device.getAllocator().flushAllocation(allocation, offset, size);
    }

    vk::DescriptorBufferInfo Buffer::getDescriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
        return vk::DescriptorBufferInfo{ buffer, offset, size };
    }

    void Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
        device.getAllocator().invalidateAllocation(allocation, offset, size);
    }

    vk::Buffer Buffer::getBuffer() const {
        return buffer;
    }

    vk::DeviceSize Buffer::getBufferSize() const {
        return bufferSize;
    }

    void *Buffer::getMappedMemory() const {
        return mapped;
    }

    uint32_t Buffer::getInstanceCount() const {
        return instanceCount;
    }

    vk::DeviceSize Buffer::getInstanceSize() const {
        return instanceSize;
    }

    vk::DeviceSize Buffer::getAlignmentSize() const {
        return alignmentSize;
    }

    vk::BufferUsageFlags Buffer::getUsageFlags() const {
        return usageFlags;
    }

}
