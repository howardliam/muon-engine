#include "muon/graphics/image.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/context.hpp"
#include "muon/utils/pretty_print.hpp"
#include "vk_mem_alloc_enums.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace muon::graphics {

Image::Image(const Spec &spec)
    : m_context(*spec.context), m_extent(spec.extent), m_format(spec.format), m_layout(spec.layout),
      m_usageFlags(spec.usageFlags), m_accessFlags(spec.accessFlags), m_stageFlags(spec.stageFlags) {
    core::expect(spec.commandBuffer != nullptr, "there must be a valid command buffer");

    CreateImage();
    CreateImageView();

    TransitionLayout(*spec.commandBuffer);

    core::debug("created image with dimensions: {}x{}, and size: {}", m_extent.width, m_extent.height, pp::PrintBytes(m_bytes));
}

Image::~Image() { core::debug("destroyed image"); }

auto Image::Get() -> vk::raii::Image & { return m_image; }
auto Image::Get() const -> const vk::raii::Image & { return m_image; }

auto Image::GetView() -> vk::raii::ImageView & { return m_imageView; }
auto Image::GetView() const -> const vk::raii::ImageView & { return m_imageView; }

auto Image::GetExtent() const -> vk::Extent2D { return m_extent; }
auto Image::GetFormat() const -> vk::Format { return m_format; }
auto Image::GetLayout() const -> vk::ImageLayout { return m_layout; }
auto Image::GetUsageFlags() const -> vk::ImageUsageFlags { return m_usageFlags; }
auto Image::GetAccessFlags() const -> vk::AccessFlags2 { return m_accessFlags; }
auto Image::GetStageFlags() const -> vk::PipelineStageFlags2 { return m_stageFlags; }
auto Image::GetDescriptorInfo() const -> const vk::DescriptorImageInfo & { return m_descriptorInfo; }

auto Image::CreateImage() -> void {
    vk::ImageCreateInfo imageCi;
    imageCi.flags = vk::ImageCreateFlags{};
    imageCi.imageType = vk::ImageType::e2D;
    imageCi.format = m_format;
    imageCi.extent = vk::Extent3D{m_extent.width, m_extent.height, 1};
    imageCi.mipLevels = 1;
    imageCi.arrayLayers = 1;
    imageCi.samples = vk::SampleCountFlagBits::e1;
    imageCi.tiling = vk::ImageTiling::eOptimal;
    imageCi.usage = m_usageFlags;
    imageCi.sharingMode = vk::SharingMode::eExclusive;
    imageCi.initialLayout = vk::ImageLayout::eUndefined;

    auto imageResult = m_context.getDevice().createImage(imageCi);
    core::expect(imageResult, "failed to create image");
    m_image = std::move(*imageResult);

    auto memoryRequirements = m_image.getMemoryRequirements();

    vma::AllocationCreateInfo allocCi;
    allocCi.usage = vma::MemoryUsage::eAutoPreferDevice;
    allocCi.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

    vma::AllocationInfo allocInfo;
    auto allocationResult = m_context.getAllocator().allocateMemory(memoryRequirements, allocCi, allocInfo);
    core::expect(allocationResult.result == vk::Result::eSuccess, "failed to allocate image memory");

    auto bindResult = m_context.getAllocator().bindImageMemory(m_allocation, m_image);
    core::expect(bindResult == vk::Result::eSuccess, "failed to bind image memory");

    m_bytes = allocInfo.size;
}

auto Image::CreateImageView() -> void {
    m_aspectMask = [](vk::Format format) -> vk::ImageAspectFlags {
        switch (format) {
            case vk::Format::eUndefined: {
                return vk::ImageAspectFlagBits::eNone;
            }

            case vk::Format::eD16Unorm:
            case vk::Format::eD32Sfloat:
            case vk::Format::eX8D24UnormPack32: {
                return vk::ImageAspectFlagBits::eDepth;
            }

            case vk::Format::eS8Uint: {
                return vk::ImageAspectFlagBits::eStencil;
            }

            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint: {
                return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            }

            default: {
                return vk::ImageAspectFlagBits::eColor;
            }
        }
    }(m_format);

    vk::ImageViewCreateInfo imageViewCi;
    imageViewCi.viewType = vk::ImageViewType::e2D;
    imageViewCi.format = m_format;
    imageViewCi.components.r = vk::ComponentSwizzle::eR;
    imageViewCi.components.g = vk::ComponentSwizzle::eG;
    imageViewCi.components.b = vk::ComponentSwizzle::eB;
    imageViewCi.components.a = vk::ComponentSwizzle::eA;
    imageViewCi.subresourceRange.aspectMask = m_aspectMask;
    imageViewCi.subresourceRange.baseMipLevel = 0;
    imageViewCi.subresourceRange.levelCount = 1;
    imageViewCi.subresourceRange.baseArrayLayer = 0;
    imageViewCi.subresourceRange.layerCount = 1;

    auto imageViewResult = m_context.getDevice().createImageView(imageViewCi);
    core::expect(imageViewResult, "failed to create image view");

    m_imageView = std::move(*imageViewResult);

    m_descriptorInfo.imageLayout = m_layout;
    m_descriptorInfo.imageView = m_imageView;
    m_descriptorInfo.sampler = nullptr;
}

auto Image::TransitionLayout(vk::raii::CommandBuffer &commandBuffer) -> void {
    vk::ImageMemoryBarrier2 barrier;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = m_aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.oldLayout = vk::ImageLayout::eUndefined;
    barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;

    barrier.newLayout = m_layout;
    barrier.dstAccessMask = m_accessFlags;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstStageMask = m_stageFlags;

    vk::DependencyInfo dependencyInfo;
    dependencyInfo.dependencyFlags = vk::DependencyFlags{};
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &barrier;
    commandBuffer.pipelineBarrier2(dependencyInfo);
}

} // namespace muon::graphics
