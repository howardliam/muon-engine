#include "muon/engine/buffer.hpp"

#include <vk_mem_alloc_enums.hpp>
#include <vk_mem_alloc_structs.hpp>
#include <print>

namespace muon::engine {

    Buffer::Buffer(
        Device &device,
        vk::DeviceSize instanceSize,
        uint32_t instanceCount,
        vk::BufferUsageFlags usageFlags,
        vk::MemoryPropertyFlags memoryPropertyFlags,
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

        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = usageFlags;

        vma::AllocationCreateInfo allocationCreateInfo;
        allocationCreateInfo.usage = vma::MemoryUsage::eAuto;
        vma::AllocationInfo allocationInfo;

        auto result = device.getAllocator().createBuffer(&bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo);
        if (result != vk::Result::eSuccess) {
            std::println("failed to allocate buffer");
        }

        memory = allocationInfo.deviceMemory;
        mapped = allocationInfo.pMappedData;
    }

    Buffer::~Buffer() {
        unmap();
        device.getAllocator().destroyBuffer(buffer, allocation);
        device.getAllocator().freeMemory(allocation);
    }

    vk::Result Buffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
        return device.getAllocator().mapMemory(allocation, &mapped);
    }

    void Buffer::unmap() {
        if (mapped == nullptr) {
            return;
        }

        device.getAllocator().unmapMemory(allocation);
        mapped = nullptr;
    }

}
