#include "muon/graphics/image.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/format/bytes.hpp"
#include "muon/graphics/context.hpp"
#include "vk_mem_alloc_enums.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace muon::graphics {

Image::Image(
    const Context &context,
    vk::raii::CommandBuffer &commandBuffer,
    vk::Extent2D extent,
    vk::Format format,
    vk::ImageLayout layout,
    vk::ImageUsageFlags usageFlags,
    vk::AccessFlags2 accessFlags,
    vk::PipelineStageFlags2 stageFlags
) : m_context{context}, m_extent{extent}, m_format{format}, m_layout{layout},
m_usageFlags{usageFlags}, m_accessFlags{accessFlags}, m_stageFlags{stageFlags} {
    createImage();
    createImageView();

    transitionLayout(commandBuffer);

    core::debug("created image with dimensions: {}x{}, and size: {}", m_extent.width, m_extent.height, format::formatBytes(m_bytes));
}

Image::~Image() { core::debug("destroyed image"); }

auto Image::get() -> vk::raii::Image & { return m_image; }
auto Image::get() const -> const vk::raii::Image & { return m_image; }

auto Image::getView() -> vk::raii::ImageView & { return m_imageView; }
auto Image::getView() const -> const vk::raii::ImageView & { return m_imageView; }

auto Image::getExtent() const -> vk::Extent2D { return m_extent; }
auto Image::getFormat() const -> vk::Format { return m_format; }
auto Image::getLayout() const -> vk::ImageLayout { return m_layout; }
auto Image::getUsageFlags() const -> vk::ImageUsageFlags { return m_usageFlags; }
auto Image::getAccessFlags() const -> vk::AccessFlags2 { return m_accessFlags; }
auto Image::getStageFlags() const -> vk::PipelineStageFlags2 { return m_stageFlags; }
auto Image::getDescriptorInfo() const -> const vk::DescriptorImageInfo & { return m_descriptorInfo; }

void Image::createImage() {
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

void Image::createImageView() {
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

void Image::transitionLayout(vk::raii::CommandBuffer &commandBuffer) {
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
