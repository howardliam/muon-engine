#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace muon::graphics {

class Image : NoCopy, NoMove {
public:
    Image(
        const Context &context,
        vk::raii::CommandBuffer &commandBuffer,
        vk::Extent2D extent,
        vk::Format format,
        vk::ImageLayout layout,
        vk::ImageUsageFlags usageFlags,
        vk::AccessFlags2 accessFlags,
        vk::PipelineStageFlags2 stageFlags
    );
    ~Image();

public:
    auto get() -> vk::raii::Image &;
    auto get() const -> const vk::raii::Image &;

    auto getView() -> vk::raii::ImageView &;
    auto getView() const -> const vk::raii::ImageView &;

    auto getExtent() const -> vk::Extent2D;
    auto getFormat() const -> vk::Format;
    auto getLayout() const -> vk::ImageLayout;
    auto getUsageFlags() const -> vk::ImageUsageFlags;
    auto getAccessFlags() const -> vk::AccessFlags2;
    auto getStageFlags() const -> vk::PipelineStageFlags2;
    auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

private:
    void createImage();
    void createImageView();
    void transitionLayout(vk::raii::CommandBuffer &commandBuffer);

private:
    const Context &m_context;

    vk::raii::Image m_image{nullptr};
    vma::Allocation m_allocation{nullptr};
    vk::raii::ImageView m_imageView{nullptr};

    vk::DeviceSize m_bytes;
    vk::Extent2D m_extent;
    vk::Format m_format;
    vk::ImageLayout m_layout;
    vk::ImageUsageFlags m_usageFlags;
    vk::AccessFlags2 m_accessFlags;
    vk::PipelineStageFlags2 m_stageFlags;
    vk::ImageAspectFlags m_aspectMask;

    vk::DescriptorImageInfo m_descriptorInfo;
};

} // namespace muon::graphics
