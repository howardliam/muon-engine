#include "muon/renderer/image.hpp"

#include <memory>
#include "muon/renderer/device.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace muon {

    Image::Image(
        Device &device,
        vk::Extent2D extent,
        vk::Format format,
        vk::ImageUsageFlags usageFlags,
        vk::ImageLayout imageLayout,
        vk::AccessFlags2 accessFlags,
        vk::PipelineStageFlags2 stageFlags
    ) : device(device), extent(extent), format(format), usageFlags(usageFlags) {
        descriptorInfo = std::make_unique<vk::DescriptorImageInfo>();
        createImage();

        auto cmd = device.beginSingleTimeCommands();
        transitionLayout(cmd, imageLayout, accessFlags, stageFlags);
        device.endSingleTimeCommands(cmd);

        MU_CORE_DEBUG("created image with dimensions: {}x{}", extent.width, extent.height);
    }

    Image::~Image() {
        device.device().destroyImageView(imageView);
        device.allocator().destroyImage(image, allocation);
        MU_CORE_DEBUG("destroyed image");
    }

    void Image::transitionLayout(
        vk::CommandBuffer cmd,
        vk::ImageLayout imageLayout,
        vk::AccessFlags2 accessFlags,
        vk::PipelineStageFlags2 stageFlags
    ) {
        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = this->imageLayout;
        barrier.srcAccessMask = this->accessFlags;
        barrier.srcStageMask = this->stageFlags;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;

        barrier.newLayout = imageLayout;
        barrier.dstAccessMask = accessFlags;
        barrier.dstStageMask = stageFlags;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectFlags;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::DependencyInfo dependencyInfo{};
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;

        cmd.pipelineBarrier2(dependencyInfo);

        this->imageLayout = imageLayout;
        this->accessFlags = accessFlags;
        this->stageFlags = stageFlags;
        descriptorInfo->imageLayout = this->imageLayout;
    }

    void Image::resize(vk::Extent2D extent) {
        device.device().destroyImageView(imageView);
        device.allocator().destroyImage(image, allocation);
        this->extent = extent;

        createImage();

        auto cmd = device.beginSingleTimeCommands();
        transitionLayout(cmd, imageLayout, accessFlags, stageFlags);
        device.endSingleTimeCommands(cmd);

        MU_CORE_DEBUG("recreated image with dimensions: {}x{}", extent.width, extent.height);
    }

    vk::Extent2D Image::getExtent() const {
        return extent;
    }

    vk::ImageLayout Image::getImageLayout() const {
        return imageLayout;
    }

    vk::Format Image::getFormat() const {
        return format;
    }

    vk::Image Image::getImage() const {
        return image;
    }

    vk::ImageView Image::getImageView() const {
        return imageView;
    }

    vk::DescriptorImageInfo *Image::getDescriptorInfo() const {
        return descriptorInfo.get();
    }

    void Image::createImage() {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = usageFlags;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.flags = vk::ImageCreateFlagBits{};

        device.createImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, vma::MemoryUsage::eGpuOnly, image, allocation);

        aspectFlags = [](vk::Format format) -> vk::ImageAspectFlags {
            switch (format) {
            case vk::Format::eUndefined:
                return {};

            case vk::Format::eD16Unorm:
            case vk::Format::eD32Sfloat:
            case vk::Format::eX8D24UnormPack32:
                return vk::ImageAspectFlagBits::eDepth;

            case vk::Format::eS8Uint:
                return vk::ImageAspectFlagBits::eStencil;

            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint:
                return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

            default:
                return vk::ImageAspectFlagBits::eColor;
            }
        }(format);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.components.r = vk::ComponentSwizzle::eR;
        viewInfo.components.g = vk::ComponentSwizzle::eG;
        viewInfo.components.b = vk::ComponentSwizzle::eB;
        viewInfo.components.a = vk::ComponentSwizzle::eA;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = device.device().createImageView(&viewInfo, nullptr, &imageView);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create image view")

        descriptorInfo->imageView = imageView;
        descriptorInfo->sampler = nullptr;
    }

    Image::Builder::Builder(Device &device) : device(device) { }

    Image::Builder &Image::Builder::setExtent(vk::Extent2D extent) {
        this->extent = extent;
        return *this;
    }

    Image::Builder &Image::Builder::setFormat(vk::Format format) {
        this->format = format;
        return *this;
    }

    Image::Builder &Image::Builder::setImageUsageFlags(vk::ImageUsageFlags imageUsageFlags) {
        this->imageUsageFlags = imageUsageFlags;
        return *this;
    }

    Image::Builder &Image::Builder::setImageLayout(vk::ImageLayout imageLayout) {
        this->imageLayout = imageLayout;
        return *this;
    }

    Image::Builder &Image::Builder::setAccessFlags(vk::AccessFlags2 accessFlags) {
        this->accessFlags = accessFlags;
        return *this;
    }

    Image::Builder &Image::Builder::setPipelineStageFlags(vk::PipelineStageFlags2 stageFlags) {
        this->stageFlags = stageFlags;
        return *this;
    }

    Image Image::Builder::build() const {
        return Image(
            device,
            extent,
            format,
            imageUsageFlags,
            imageLayout,
            accessFlags,
            stageFlags
        );
    }

    std::unique_ptr<Image> Image::Builder::buildUniquePtr() const {
        return std::make_unique<Image>(
            device,
            extent,
            format,
            imageUsageFlags,
            imageLayout,
            accessFlags,
            stageFlags
        );
    }

}
