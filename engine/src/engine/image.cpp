#include "muon/engine/image.hpp"
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>

namespace muon::engine {

    Image::Image(
        Device &device,
        vk::Extent2D extent,
        vk::ImageLayout layout,
        vk::Format format,
        vk::ImageUsageFlags usageFlags
    ) : device(device), extent(extent), imageLayout(layout), format(format), usageFlags(usageFlags) {
        createImage();
        transitionImage();
    }

    Image::~Image() {
        device.getDevice().destroyImageView(imageView);
        device.getAllocator().destroyImage(image, allocation);
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

    vk::DescriptorImageInfo Image::getDescriptorInfo() const {
        return vk::DescriptorImageInfo{
            nullptr,
            imageView,
            imageLayout,
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
    }

    void Image::transitionImage() {
        auto getDstInfo = [](vk::ImageLayout layout) -> std::tuple<vk::AccessFlags, vk::PipelineStageFlags> {
            switch (layout) {
                case vk::ImageLayout::eGeneral:
                    return {
                        vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
                        vk::PipelineStageFlagBits::eComputeShader
                    };

                case vk::ImageLayout::eShaderReadOnlyOptimal:
                    return {
                        vk::AccessFlagBits::eShaderRead,
                        vk::PipelineStageFlagBits::eFragmentShader
                    };

                case vk::ImageLayout::eColorAttachmentOptimal:
                    return {
                        vk::AccessFlagBits::eColorAttachmentWrite,
                        vk::PipelineStageFlagBits::eColorAttachmentOutput
                    };

                default:
                    return {{}, {}};
            }
        };

        auto [accessFlags, stageFlags] = getDstInfo(imageLayout);

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = imageLayout;
        barrier.srcAccessMask = vk::AccessFlagBits{};
        barrier.dstAccessMask = accessFlags;
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
            stageFlags,
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

}
