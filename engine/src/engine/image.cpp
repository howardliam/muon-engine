#include "muon/engine/image.hpp"

#include <memory>
#include <stdexcept>
#include "muon/engine/device.hpp"
#include "muon/log/logger.hpp"

namespace muon::engine {

    Image::Image(
        Device &device,
        vk::Extent2D extent,
        vk::Format format,
        vk::ImageUsageFlags usageFlags,
        const State &state
    ) : device(device), extent(extent), format(format), usageFlags(usageFlags), state(state) {
        createImage();
        log::globalLogger->debug("created image with dimensions: {}x{}", extent.width, extent.height);
    }

    Image::~Image() {
        device.getDevice().destroyImageView(imageView);
        device.getAllocator().destroyImage(image, allocation);
        log::globalLogger->debug("destroyed image");
    }

    void Image::transitionLayout(
        vk::CommandBuffer cmd,
        const State &newState
    ) {
        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = state.imageLayout;
        barrier.srcStageMask = state.stageFlags;
        barrier.srcAccessMask = state.accessFlags;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;

        barrier.newLayout = newState.imageLayout;
        barrier.dstStageMask = newState.stageFlags;
        barrier.dstAccessMask = newState.accessFlags;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::DependencyInfo dependencyInfo{};
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;

        cmd.pipelineBarrier2(dependencyInfo);

        state = newState;
        transitioned = true;
    }

    vk::Extent2D Image::getExtent() const {
        return extent;
    }

    vk::ImageLayout Image::getImageLayout() const {
        return state.imageLayout;
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

    vk::DescriptorImageInfo Image::getDescriptorInfo() const {
        return vk::DescriptorImageInfo{
            nullptr,
            imageView,
            state.imageLayout,
        };
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

        auto aspectMask = [](vk::ImageLayout layout, vk::Format format) -> vk::ImageAspectFlags {
            switch (layout) {
            case vk::ImageLayout::eGeneral:
            case vk::ImageLayout::eColorAttachmentOptimal:
                return vk::ImageAspectFlagBits::eColor;

            case vk::ImageLayout::eDepthAttachmentOptimal:
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                if (format == vk::Format::eD16Unorm || format == vk::Format::eD32Sfloat) {
                    return vk::ImageAspectFlagBits::eDepth;
                }
                return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;

            default:
                return {};
            }
        }(state.imageLayout, format);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.components.r = vk::ComponentSwizzle::eR;
        viewInfo.components.g = vk::ComponentSwizzle::eG;
        viewInfo.components.b = vk::ComponentSwizzle::eB;
        viewInfo.components.a = vk::ComponentSwizzle::eA;
        viewInfo.subresourceRange.aspectMask = aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = device.getDevice().createImageView(&viewInfo, nullptr, &imageView);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create image view");
        }

        vk::ImageMemoryBarrier2 barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
        barrier.srcAccessMask = vk::AccessFlags2{};
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;

        barrier.newLayout = state.imageLayout;
        barrier.dstStageMask = state.stageFlags;
        barrier.dstAccessMask = state.accessFlags;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::DependencyInfo dependencyInfo{};
        dependencyInfo.imageMemoryBarrierCount = 1;
        dependencyInfo.pImageMemoryBarriers = &barrier;

        auto cmd = device.beginSingleTimeCommands();

        cmd.pipelineBarrier2(dependencyInfo);

        device.endSingleTimeCommands(cmd);

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
        return Image(device, extent, format, imageUsageFlags, State{imageLayout, accessFlags, stageFlags});
    }

    std::unique_ptr<Image> Image::Builder::buildUniquePtr() const {
        return std::make_unique<Image>(device, extent, format, imageUsageFlags, State{imageLayout, accessFlags, stageFlags});
    }

}
