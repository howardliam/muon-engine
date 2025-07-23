#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace muon::graphics {

class Image : NoCopy, NoMove {
public:
    struct Spec {
        const Context *context{nullptr};
        vk::raii::CommandBuffer *commandBuffer{nullptr};
        vk::Extent2D extent;
        vk::Format format;
        vk::ImageLayout layout;
        vk::ImageUsageFlags usageFlags;
        vk::AccessFlags2 accessFlags;
        vk::PipelineStageFlags2 stageFlags;
    };

public:
    Image(const Spec &spec);
    ~Image();

public:
    auto Get() -> vk::raii::Image &;
    auto Get() const -> const vk::raii::Image &;

    auto GetView() -> vk::raii::ImageView &;
    auto GetView() const -> const vk::raii::ImageView &;

    auto GetExtent() const -> vk::Extent2D;
    auto GetFormat() const -> vk::Format;
    auto GetLayout() const -> vk::ImageLayout;
    auto GetUsageFlags() const -> vk::ImageUsageFlags;
    auto GetAccessFlags() const -> vk::AccessFlags2;
    auto GetStageFlags() const -> vk::PipelineStageFlags2;
    auto GetDescriptorInfo() const -> const vk::DescriptorImageInfo &;

private:
    auto CreateImage() -> void;
    auto CreateImageView() -> void;
    auto TransitionLayout(vk::raii::CommandBuffer &commandBuffer) -> void;

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
