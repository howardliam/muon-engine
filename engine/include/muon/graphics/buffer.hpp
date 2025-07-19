#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/graphics/context.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Buffer : NoCopy {
public:
    struct Spec {
        const Context *context{nullptr};
        VkDeviceSize instanceSize{};
        uint32_t instanceCount{};
        VkBufferUsageFlags usageFlags{};
        VmaMemoryUsage memoryUsage{VMA_MEMORY_USAGE_AUTO};
        VkDeviceSize minOffsetAlignment{1};
    };

    static constexpr bool transientResource = true;

public:
    Buffer(const Spec &spec);
    ~Buffer();

    Buffer(Buffer &&other) noexcept;
    auto operator=(Buffer &&other) noexcept -> Buffer &;

    [[nodiscard]] auto Map() -> VkResult;
    auto Unmap() -> void;

    auto Write(const void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;
    auto Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;
    auto Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) -> void;

public:
    auto Get() const -> VkBuffer;
    auto GetSize() const -> VkDeviceSize;
    auto GetMappedMemory() const -> void *;
    auto GetDeviceAddress() const -> VkDeviceAddress;

    auto GetInstanceCount() const -> uint32_t;
    auto GetInstanceSize() const -> VkDeviceSize;

    auto GetDescriptorInfo() const -> const VkDescriptorBufferInfo &;

private:
    const Context &m_context;

    VkDeviceSize m_instanceSize{};
    uint32_t m_instanceCount{};
    VkDeviceSize m_alignmentSize{};
    VkDeviceSize m_size{};

    VkBufferUsageFlags m_usageFlags{};

    VkBuffer m_buffer{nullptr};
    VmaAllocation m_allocation{nullptr};
    void *m_mapped{nullptr};
    VkDeviceAddress m_deviceAddress{};

    VkDescriptorBufferInfo m_descriptorInfo{};
};

} // namespace muon::graphics
