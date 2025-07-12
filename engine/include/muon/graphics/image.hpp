#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/device_context.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Image : NoCopy, NoMove {
public:
    struct Spec {
        const DeviceContext *device{nullptr};
        VkExtent2D extent;
        VkFormat format;
        VkImageLayout layout;
        VkImageUsageFlags usageFlags;
        VkAccessFlags2 accessFlags;
        VkPipelineStageFlags2 stageFlags;
        VkCommandBuffer cmd{nullptr};
    };

public:
    Image(const Spec &spec);
    ~Image();

public:
    [[nodiscard]] auto GetExtent() const -> VkExtent2D;
    [[nodiscard]] auto GetFormat() const -> VkFormat;
    [[nodiscard]] auto GetLayout() const -> VkImageLayout;
    [[nodiscard]] auto GetUsageFlags() const -> VkImageUsageFlags;
    [[nodiscard]] auto GetAccessFlags() const -> VkAccessFlags2;
    [[nodiscard]] auto GetStageFlags() const -> VkPipelineStageFlags2;

    [[nodiscard]] auto Get() const -> VkImage;
    [[nodiscard]] auto GetView() const -> VkImageView;
    [[nodiscard]] auto GetDescriptorInfo() const -> const VkDescriptorImageInfo &;

private:
    auto CreateImage() -> void;
    auto CreateImageView() -> void;
    auto TransitionLayout(VkCommandBuffer cmd) -> void;

private:
    const DeviceContext &m_device;

    VkDeviceSize m_bytes;
    VkExtent2D m_extent;
    VkFormat m_format;
    VkImageLayout m_layout;
    VkImageUsageFlags m_usageFlags;
    VkAccessFlags2 m_accessFlags;
    VkPipelineStageFlags2 m_stageFlags;
    VkImageAspectFlags m_aspectMask;

    VkImage m_image{nullptr};
    VmaAllocation m_allocation{nullptr};
    VkImageView m_imageView{nullptr};

    VkDescriptorImageInfo m_descriptorInfo{};
};

} // namespace muon::graphics
