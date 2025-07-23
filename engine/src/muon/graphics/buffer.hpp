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

    auto Map() -> std::expected<void, vk::Result>;
    auto Unmap() -> void;

    auto Write(const void *data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) -> void;
    auto Flush(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) -> void;
    auto Invalidate(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0) -> void;

public:
    auto Get() -> vk::raii::Buffer &;
    auto Get() const -> const vk::raii::Buffer &;

    auto GetSize() const -> vk::DeviceSize;
    auto GetMappedMemory() const -> void *;
    auto GetDeviceAddress() const -> vk::DeviceAddress;

    auto GetInstanceCount() const -> uint32_t;
    auto GetInstanceSize() const -> vk::DeviceSize;

    auto GetDescriptorInfo() const -> const vk::DescriptorBufferInfo &;

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
