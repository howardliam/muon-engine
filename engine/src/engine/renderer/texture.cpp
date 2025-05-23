#include "muon/engine/renderer/texture.hpp"

#include "muon/engine/renderer/device.hpp"
#include "muon/engine/renderer/buffer.hpp"

namespace mu {

    Texture::Texture(
        Device &device,
        const std::vector<uint8_t> &textureData,
        uint32_t channels,
        vk::Extent2D extent
    ) : device(device), extent(extent) {
        createTexture(channels);
        prepareForCopying();
        copyToTexture(textureData, channels);
        prepareForShader();
    }

    Texture::~Texture() {
        device.getDevice().destroySampler(sampler);
        device.getDevice().destroyImageView(imageView);
        device.getAllocator().destroyImage(image, allocation);
    }

    vk::DescriptorImageInfo Texture::getDescriptorInfo() const {
        return vk::DescriptorImageInfo{
            sampler,
            imageView,
            vk::ImageLayout::eShaderReadOnlyOptimal,
        };
    }

    void Texture::createTexture(uint32_t channels) {
        auto format = [](uint32_t channels) {
            switch (channels) {
            case 3:
                return vk::Format::eR8G8B8Srgb;

            case 4:
                return vk::Format::eR8G8B8A8Srgb;

            default:
                return vk::Format::eUndefined;
            }
        }(channels);

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
        imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
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

        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.compareOp = vk::CompareOp::eNever;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.maxAnisotropy = 4.0f;
        samplerInfo.anisotropyEnable = false;
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;

        result = device.getDevice().createSampler(&samplerInfo, nullptr, &sampler);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create sampler");
        }
    }

    void Texture::prepareForCopying() {
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eUndefined;
        barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits{};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
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
            vk::PipelineStageFlagBits::eTransfer,
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

    void Texture::copyToTexture(
        const std::vector<uint8_t> &textureData,
        uint32_t channels
    ) {
        uint32_t pixelSize = channels;
        uint32_t pixelCount = static_cast<uint32_t>(textureData.size()) / pixelSize;

        Buffer stagingBuffer(
            device,
            pixelSize,
            pixelCount,
            vk::BufferUsageFlagBits::eTransferSrc,
            vma::MemoryUsage::eCpuOnly
        );

        auto _ = stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)textureData.data());

        device.copyBufferToImage(stagingBuffer.getBuffer(), image, extent.width, extent.height, 1);
    }

    void Texture::prepareForShader() {
        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
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
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader,
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

    Texture::Builder::Builder(Device &device) : device(device) {}

    Texture::Builder &Texture::Builder::setExtent(vk::Extent2D extent) {
        this->extent = extent;
        return *this;
    }

    Texture Texture::Builder::build(const std::vector<uint8_t> &textureData, uint32_t channels) const {
        return Texture(device, textureData, channels, extent);
    }

    std::unique_ptr<Texture> Texture::Builder::buildUniquePtr(const std::vector<uint8_t> &textureData, uint32_t channels) const {
        return std::make_unique<Texture>(device, textureData, channels, extent);
    }

}
