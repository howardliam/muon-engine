#include "muon/engine/image.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>

namespace muon::engine {

    Image::Image(
        Device &device,
        vk::Extent2D extent,
        vk::Format format,
        vk::ImageUsageFlags usageFlags,
        const State &state
    ) :
        device(device),
        extent(extent),
        format(format),
        usageFlags(usageFlags),
        state(state)
    {
        createImage();
    }

    Image::~Image() {
        device.getDevice().destroyImageView(imageView);
        device.getAllocator().destroyImage(image, allocation);
    }

    void Image::transition(
        vk::CommandBuffer commandBuffer,
        const State &newState
    ) {
        if (transitioned) {
            return;
        }

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = state.imageLayout;
        barrier.newLayout = newState.imageLayout;
        barrier.srcAccessMask = state.accessFlags;
        barrier.dstAccessMask = newState.accessFlags;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        commandBuffer.pipelineBarrier(
            state.pipelineStageFlags,
            newState.pipelineStageFlags,
            vk::DependencyFlagBits{},
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        oldState = state;
        state = newState;
        transitioned = true;
    }

    void Image::detransition(vk::CommandBuffer commandBuffer) {
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = state.imageLayout;
        barrier.newLayout = oldState.imageLayout;
        barrier.srcAccessMask = state.accessFlags;
        barrier.dstAccessMask = oldState.accessFlags;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        commandBuffer.pipelineBarrier(
            state.pipelineStageFlags,
            oldState.pipelineStageFlags,
            vk::DependencyFlagBits{},
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        state = oldState;
        transitioned = false;
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

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;
        viewInfo.components.r = vk::ComponentSwizzle::eR;
        viewInfo.components.g = vk::ComponentSwizzle::eG;
        viewInfo.components.b = vk::ComponentSwizzle::eB;
        viewInfo.components.a = vk::ComponentSwizzle::eA;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = device.getDevice().createImageView(&viewInfo, nullptr, &imageView);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create image view");
        }

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = state.imageLayout;
        barrier.srcAccessMask = vk::AccessFlagBits{};
        barrier.dstAccessMask = state.accessFlags;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        auto commandBuffer = device.beginSingleTimeCommands();

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe,
            state.pipelineStageFlags,
            vk::DependencyFlagBits{},
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
        );

        device.endSingleTimeCommands(commandBuffer);
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

    Image::Builder &Image::Builder::setAccessFlags(vk::AccessFlags accessFlags) {
        this->accessFlags = accessFlags;
        return *this;
    }

    Image::Builder &Image::Builder::setPipelineStageFlags(vk::PipelineStageFlags pipelineStageFlags) {
        this->pipelineStageFlags = pipelineStageFlags;
        return *this;
    }

    Image Image::Builder::build() const {
        return Image(device, extent, format, imageUsageFlags, State{imageLayout, accessFlags, pipelineStageFlags});
    }

    std::unique_ptr<Image> Image::Builder::buildUniquePtr() const {
        return std::make_unique<Image>(device, extent, format, imageUsageFlags, State{imageLayout, accessFlags, pipelineStageFlags});
    }

}
