#include "muon/graphics/buffer.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/format/bytes.hpp"
#include "muon/utils/alignment.hpp"
#include "vk_mem_alloc.hpp"
#include "vk_mem_alloc_enums.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <cstring>

namespace muon::graphics {

Buffer::Buffer(
    const Context &context,
    vk::DeviceSize instanceSize,
    uint32_t instanceCount,
    vk::BufferUsageFlags bufferUsage,
    vma::MemoryUsage memoryUsage,
    vk::DeviceSize minOffsetAlignment
) : m_context(context), m_instanceSize(instanceSize), m_instanceCount(instanceCount), m_usageFlags(bufferUsage) {
    m_alignmentSize = minOffsetAlignment > 0 ? utils::alignment(m_instanceSize, minOffsetAlignment) : m_instanceSize;
    m_size = m_alignmentSize * m_instanceCount;

    vk::BufferCreateInfo bufferCi;
    bufferCi.usage = bufferUsage;
    bufferCi.size = m_size;
    bufferCi.sharingMode = vk::SharingMode::eExclusive;

    vma::AllocationCreateInfo allocationCi;
    allocationCi.usage = memoryUsage;
    allocationCi.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;

    vma::AllocationInfo allocationInfo;

    auto bufferResult = m_context.getAllocator().createBuffer(bufferCi, allocationCi, allocationInfo);
    core::expect(bufferResult.result == vk::Result::eSuccess, "failed to create buffer");

    auto [buffer, allocation] = bufferResult.value;
    m_buffer = vk::raii::Buffer{m_context.getDevice(), buffer};

    m_allocation = allocation;

    if (m_usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        vk::BufferDeviceAddressInfo bdaInfo;
        bdaInfo.buffer = m_buffer;

        m_deviceAddress = m_context.getDevice().getBufferAddress(bdaInfo);
    }

    m_descriptorInfo.buffer = m_buffer;
    m_descriptorInfo.range = m_size;
    m_descriptorInfo.offset = 0;

    core::debug("created buffer with size: {}", format::formatBytes(m_size));
}

Buffer::~Buffer() {
    unmap();
    m_context.getAllocator().destroyBuffer(m_buffer, m_allocation);
    core::debug("destroyed buffer");
}

Buffer::Buffer(Buffer &&other) noexcept
    : m_context{other.m_context}, m_instanceSize{other.m_instanceSize}, m_instanceCount{other.m_instanceCount},
      m_alignmentSize{other.m_alignmentSize}, m_size{other.m_size}, m_usageFlags{other.m_usageFlags},
      m_allocation{other.m_allocation}, m_deviceAddress{other.m_deviceAddress}, m_descriptorInfo{other.m_descriptorInfo} {

    m_buffer = std::move(other.m_buffer);

    other.unmap();

    other.m_buffer = nullptr;
    other.m_allocation = nullptr;
    other.m_descriptorInfo = vk::DescriptorBufferInfo{};
}

auto Buffer::operator=(Buffer &&other) noexcept -> Buffer & {
    if (this != &other) {
        other.unmap();

        m_instanceSize = other.m_instanceSize;
        m_instanceCount = other.m_instanceCount;
        m_alignmentSize = other.m_alignmentSize;
        m_size = other.m_size;

        m_usageFlags = other.m_usageFlags;

        m_buffer = std::move(other.m_buffer);
        m_allocation = other.m_allocation;
        m_deviceAddress = other.m_deviceAddress;

        m_descriptorInfo = other.m_descriptorInfo;
    }

    return *this;
}

auto Buffer::map() -> std::expected<void, vk::Result> {
    auto result = m_context.getAllocator().mapMemory(m_allocation);
    if (result.result != vk::Result::eSuccess) {
        return std::unexpected(result.result);
    }

    m_mapped = result.value;

    return {};
}

auto Buffer::unmap() -> void {
    if (m_mapped == nullptr) {
        return;
    }

    m_context.getAllocator().unmapMemory(m_allocation);
    m_mapped = nullptr;
}

auto Buffer::write(const void *data, vk::DeviceSize size, vk::DeviceSize offset) -> void {
    if (size == vk::WholeSize) {
        std::memcpy(m_mapped, data, m_size);
    } else {
        auto memoryOffset = static_cast<uint8_t *>(m_mapped);
        memoryOffset += offset;
        std::memcpy(memoryOffset, data, size);
    }
}

auto Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) -> void {
    vmaFlushAllocation(m_context.getAllocator(), m_allocation, offset, size);
}

auto Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) -> void {
    vmaInvalidateAllocation(m_context.getAllocator(), m_allocation, offset, size);
}

auto Buffer::get() -> vk::raii::Buffer & { return m_buffer; }
auto Buffer::get() const -> const vk::raii::Buffer & { return m_buffer; }

auto Buffer::getSize() const -> vk::DeviceSize { return m_size; }

auto Buffer::getMappedMemory() const -> void * { return m_mapped; }

auto Buffer::getDeviceAddress() const -> VkDeviceAddress {
    core::expect(
        m_usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress, "buffer must be created with shader context address usage"
    );
    return m_deviceAddress;
}

auto Buffer::getInstanceCount() const -> uint32_t { return m_instanceCount; }

auto Buffer::getInstanceSize() const -> vk::DeviceSize { return m_instanceSize; }

auto Buffer::getDescriptorInfo() const -> const vk::DescriptorBufferInfo & { return m_descriptorInfo; }

} // namespace muon::graphics
