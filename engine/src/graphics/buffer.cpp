#include "muon/graphics/buffer.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/utils/pretty_print.hpp"
#include <cstring>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::graphics {

    Buffer::Buffer(const Spec &spec) : m_device(*spec.device), m_usageFlags(spec.usageFlags) {
        auto getAlignment = [](VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) -> VkDeviceSize {
            if (minOffsetAlignment > 0) {
                return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
            }
            return instanceSize;
        };

        m_alignmentSize = getAlignment(spec.instanceSize, spec.minOffsetAlignment);
        m_size = m_alignmentSize * spec.instanceCount;

        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = spec.usageFlags;
        bufferCreateInfo.size = m_size;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.requiredFlags = spec.memoryPropertyFlags;
        allocationCreateInfo.usage = spec.memoryUsage;
        allocationCreateInfo.flags = spec.allocationFlags;

        VmaAllocationInfo allocationInfo{};

        auto result = vmaCreateBuffer(
            m_device.GetAllocator(),
            &bufferCreateInfo,
            &allocationCreateInfo,
            &m_buffer,
            &m_allocation,
            &allocationInfo
        );
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create buffer");

        if (m_usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfo addressInfo{};
            addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            addressInfo.buffer = m_buffer;

            m_deviceAddress = vkGetBufferDeviceAddress(m_device.GetDevice(), &addressInfo);
        }

        m_descriptorInfo.buffer = m_buffer;
        m_descriptorInfo.range = m_size;
        m_descriptorInfo.offset = 0;

        MU_CORE_DEBUG("created buffer with size: {}", pp::PrintBytes(m_size));
    }

    Buffer::~Buffer() {
        Unmap();
        vmaDestroyBuffer(m_device.GetAllocator(), m_buffer, m_allocation);
        MU_CORE_DEBUG("destroyed buffer");
    }

    [[nodiscard]] auto Buffer::Map() -> VkResult {
        return vmaMapMemory(m_device.GetAllocator(), m_allocation, &m_mapped);
    }

    auto Buffer::Unmap() -> void {
        if (m_mapped == nullptr) { return; }
        vmaUnmapMemory(m_device.GetAllocator(), m_allocation);
        m_mapped = nullptr;
    }

    auto Buffer::Write(const void *data, VkDeviceSize size, VkDeviceSize offset) -> void {
        if (size == VK_WHOLE_SIZE) {
            std::memcpy(m_mapped, data, m_size);
        } else {
            auto memoryOffset = static_cast<uint8_t *>(m_mapped);
            memoryOffset += offset;
            std::memcpy(memoryOffset, data, size);
        }
    }

    auto Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) -> void {
        vmaFlushAllocation(m_device.GetAllocator(), m_allocation, offset, size);
    }

    auto Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset) -> void {
        vmaInvalidateAllocation(m_device.GetAllocator(), m_allocation, offset, size);
    }

    auto Buffer::Get() const -> VkBuffer {
        return m_buffer;
    }

    auto Buffer::GetSize() const -> VkDeviceSize {
        return m_size;
    }

    auto Buffer::GetMappedMemory() const -> void * {
        return m_mapped;
    }

    auto Buffer::GetDeviceAddress() const -> VkDeviceAddress {
        MU_CORE_ASSERT(m_usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, "buffer must be created with shader device address usage");
        return m_deviceAddress;
    }

    auto Buffer::GetInstanceCount() const -> uint32_t {
        return m_instanceCount;
    }

    auto Buffer::GetInstanceSize() const -> VkDeviceSize {
        return m_instanceSize;
    }

    auto Buffer::GetDescriptorInfo() const -> const VkDescriptorBufferInfo & {
        return m_descriptorInfo;
    }

}
