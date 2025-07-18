#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Image : NoCopy, NoMove {
public:
    struct Spec {
        const Context *context{nullptr};
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
    auto GetExtent() const -> VkExtent2D;
    auto GetFormat() const -> VkFormat;
    auto GetLayout() const -> VkImageLayout;
    auto GetUsageFlags() const -> VkImageUsageFlags;
    auto GetAccessFlags() const -> VkAccessFlags2;
    auto GetStageFlags() const -> VkPipelineStageFlags2;

    auto Get() const -> VkImage;
    auto GetView() const -> VkImageView;
    auto GetDescriptorInfo() const -> const VkDescriptorImageInfo &;

private:
    auto CreateImage() -> void;
    auto CreateImageView() -> void;
    auto TransitionLayout(VkCommandBuffer cmd) -> void;

private:
    const Context &m_context;

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
