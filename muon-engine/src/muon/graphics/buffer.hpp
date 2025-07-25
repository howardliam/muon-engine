#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/graphics/context.hpp"
#include "vk_mem_alloc_enums.hpp"
#include "vulkan/vulkan.hpp"

#include <expected>

namespace muon::graphics {

class Buffer : NoCopy {
public:
    struct Spec {
        const Context *context{nullptr};
        vk::DeviceSize instanceSize{};
        uint32_t instanceCount{};
        vk::BufferUsageFlags usageFlags{};
        vma::MemoryUsage memoryUsage{vma::MemoryUsage::eAuto};
        vk::DeviceSize minOffsetAlignment{1};
    };

public:
    Buffer(const Spec &spec);
    ~Buffer();

    Buffer(Buffer &&other) noexcept;
    auto operator=(Buffer &&other) noexcept -> Buffer &;

    auto map() -> std::expected<void, vk::Result>;
    auto unmap() -> void;

    auto write(const void *data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) -> void;
    auto flush(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) -> void;
    auto invalidate(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) -> void;

public:
    auto get() -> vk::raii::Buffer &;
    auto get() const -> const vk::raii::Buffer &;

    auto getSize() const -> vk::DeviceSize;
    auto getMappedMemory() const -> void *;
    auto getDeviceAddress() const -> vk::DeviceAddress;

    auto getInstanceCount() const -> uint32_t;
    auto getInstanceSize() const -> vk::DeviceSize;

    auto getDescriptorInfo() const -> const vk::DescriptorBufferInfo &;

private:
    const Context &m_context;

    vk::DeviceSize m_instanceSize;
    uint32_t m_instanceCount;
    vk::DeviceSize m_alignmentSize;
    vk::DeviceSize m_size;

    vk::BufferUsageFlags m_usageFlags;

    vk::raii::Buffer m_buffer{nullptr};
    vma::Allocation m_allocation;
    void *m_mapped{nullptr};
    vk::DeviceAddress m_deviceAddress;

    vk::DescriptorBufferInfo m_descriptorInfo;
};

} // namespace muon::graphics
