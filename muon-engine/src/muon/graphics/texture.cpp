#include "muon/graphics/texture.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"
#include "muon/utils/pretty_print.hpp"
#include "vk_mem_alloc_enums.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace muon::graphics {

Texture::Texture(const Spec &spec) : m_context{spec.context}, m_extent{spec.extent}, m_format{spec.format} {
    createImage();
    createImageView();
    createSampler();

    m_descriptorInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    m_descriptorInfo.imageView = m_imageView;
    m_descriptorInfo.sampler = m_sampler;

    uploadData(spec.commandBuffer, spec.uploadBuffers, spec.textureData, spec.pixelSize);

    core::debug("created texture with dimensions: {}x{}, and size: {}", m_extent.width, m_extent.height, pp::printBytes(m_size));
}

Texture::~Texture() {
    m_context.getAllocator().destroyImage(*m_image, m_allocation);
    core::debug("destroyed texture");
}

Texture::Texture(Texture &&other)
    : m_context{other.m_context}, m_size{other.m_size}, m_extent{other.m_extent}, m_format{other.m_format},
      m_image{std::move(other.m_image)}, m_allocation{std::move(other.m_allocation)}, m_imageView{std::move(other.m_imageView)},
      m_sampler{std::move(other.m_sampler)}, m_descriptorInfo{other.m_descriptorInfo} {}

auto Texture::operator=(Texture &&other) -> Texture & {
    if (this != &other) {
        m_size = other.m_size;
        m_extent = other.m_extent;
        m_format = other.m_format;
        m_image = std::move(other.m_image);
        m_allocation = std::move(other.m_allocation);
        m_imageView = std::move(other.m_imageView);
        m_sampler = std::move(other.m_sampler);
        m_descriptorInfo = other.m_descriptorInfo;
    }

    return *this;
}

auto Texture::get() -> vk::raii::Image & { return m_image; }
auto Texture::get() const -> const vk::raii::Image & { return m_image; }

auto Texture::getView() -> vk::raii::ImageView & { return m_imageView; }
auto Texture::getView() const -> const vk::raii::ImageView & { return m_imageView; }

auto Texture::getSampler() -> vk::raii::Sampler & { return m_sampler; }
auto Texture::getSampler() const -> const vk::raii::Sampler & { return m_sampler; }

auto Texture::getDescriptorInfo() const -> const vk::DescriptorImageInfo & { return m_descriptorInfo; }

auto Texture::createImage() -> void {
    vk::ImageCreateInfo imageCi;
    imageCi.extent = vk::Extent3D{m_extent.width, m_extent.height, 1};
    imageCi.imageType = vk::ImageType::e2D;
    imageCi.mipLevels = 1;
    imageCi.arrayLayers = 1;
    imageCi.format = m_format;
    imageCi.tiling = vk::ImageTiling::eOptimal;
    imageCi.initialLayout = vk::ImageLayout::eUndefined;
    imageCi.usage =
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
    imageCi.samples = vk::SampleCountFlagBits::e1;
    imageCi.flags = vk::ImageCreateFlags{};
    imageCi.sharingMode = vk::SharingMode::eConcurrent;

    std::array<uint32_t, 2> indices = {
        m_context.getGraphicsQueue().getFamilyIndex(), m_context.getTransferQueue().getFamilyIndex()
    };
    imageCi.queueFamilyIndexCount = indices.size();
    imageCi.pQueueFamilyIndices = indices.data();

    vma::AllocationCreateInfo allocCi;
    allocCi.usage = vma::MemoryUsage::eAuto;
    allocCi.flags = vma::AllocationCreateFlagBits::eDedicatedMemory;

    vma::AllocationInfo allocInfo{};

    auto [result, data] = m_context.getAllocator().createImage(imageCi, allocCi, &allocInfo);
    core::expect(result == vk::Result::eSuccess, "failed to create texture image");

    auto [image, allocation] = data;
    m_image = vk::raii::Image{m_context.getDevice(), image};
    m_allocation = allocation;

    m_size = allocInfo.size;
}

auto Texture::createImageView() -> void {
    vk::ImageViewCreateInfo imageViewCi;
    imageViewCi.image = *m_image;
    imageViewCi.viewType = vk::ImageViewType::e2D;
    imageViewCi.format = m_format;
    imageViewCi.components.r = vk::ComponentSwizzle::eR;
    imageViewCi.components.g = vk::ComponentSwizzle::eG;
    imageViewCi.components.b = vk::ComponentSwizzle::eB;
    imageViewCi.components.a = vk::ComponentSwizzle::eA;
    imageViewCi.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageViewCi.subresourceRange.baseMipLevel = 0;
    imageViewCi.subresourceRange.levelCount = 1;
    imageViewCi.subresourceRange.baseArrayLayer = 0;
    imageViewCi.subresourceRange.layerCount = 1;

    auto imageViewResult = m_context.getDevice().createImageView(imageViewCi);
    core::expect(imageViewResult, "failed to create texture image view");
    m_imageView = std::move(*imageViewResult);
}

auto Texture::createSampler() -> void {
    vk::SamplerCreateInfo samplerCi;
    samplerCi.minFilter = vk::Filter::eLinear;
    samplerCi.magFilter = vk::Filter::eLinear;
    samplerCi.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerCi.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerCi.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerCi.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerCi.compareEnable = false;
    samplerCi.compareOp = vk::CompareOp::eNever;
    samplerCi.mipLodBias = 0.0;
    samplerCi.minLod = 0.0;
    samplerCi.maxLod = 0.0;
    samplerCi.anisotropyEnable = false;
    samplerCi.maxAnisotropy = 4.0;
    samplerCi.borderColor = vk::BorderColor::eFloatOpaqueWhite;

    auto samplerResult = m_context.getDevice().createSampler(samplerCi);
    core::expect(samplerResult, "failed to create texture samper");
    m_sampler = std::move(*samplerResult);
}

auto Texture::uploadData(
    vk::raii::CommandBuffer &commandBuffer, std::deque<Buffer> *uploadBuffers, const std::vector<uint8_t> &textureData,
    uint32_t pixelSize
) -> void {
    vk::ImageMemoryBarrier2 copyImb;
    copyImb.image = m_image;
    copyImb.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    copyImb.subresourceRange.baseMipLevel = 0;
    copyImb.subresourceRange.levelCount = 1;
    copyImb.subresourceRange.baseArrayLayer = 0;
    copyImb.subresourceRange.layerCount = 1;

    copyImb.oldLayout = vk::ImageLayout::eUndefined;
    copyImb.srcAccessMask = vk::AccessFlagBits2::eNone;
    copyImb.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    copyImb.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;

    copyImb.newLayout = vk::ImageLayout::eTransferDstOptimal;
    copyImb.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
    copyImb.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    copyImb.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;

    vk::DependencyInfo copyDi;
    copyDi.dependencyFlags = vk::DependencyFlags{};
    copyDi.imageMemoryBarrierCount = 1;
    copyDi.pImageMemoryBarriers = &copyImb;
    commandBuffer.pipelineBarrier2(copyDi);

    Buffer::Spec stagingSpec{m_context};
    stagingSpec.instanceSize = pixelSize;
    stagingSpec.instanceCount = textureData.size() / pixelSize;
    stagingSpec.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    Buffer &stagingBuffer = uploadBuffers->emplace_back(stagingSpec);

    auto result = stagingBuffer.map();
    core::expect(result, "failed to map texture staging buffer");

    stagingBuffer.write(textureData.data());

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D{0, 0, 0};
    region.imageExtent = vk::Extent3D{m_extent.width, m_extent.height, 1};

    commandBuffer.copyBufferToImage(stagingBuffer.get(), m_image, vk::ImageLayout::eTransferDstOptimal, {region});

    vk::ImageMemoryBarrier2 shaderImb;
    shaderImb.image = m_image;
    shaderImb.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    shaderImb.subresourceRange.baseMipLevel = 0;
    shaderImb.subresourceRange.levelCount = 1;
    shaderImb.subresourceRange.baseArrayLayer = 0;
    shaderImb.subresourceRange.layerCount = 1;

    shaderImb.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    shaderImb.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
    shaderImb.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    shaderImb.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;

    shaderImb.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    shaderImb.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
    shaderImb.dstQueueFamilyIndex = m_context.getGraphicsQueue().getFamilyIndex();
    shaderImb.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader | vk::PipelineStageFlagBits2::eComputeShader;

    vk::DependencyInfo shaderDi;
    shaderDi.dependencyFlags = vk::DependencyFlags{};
    shaderDi.imageMemoryBarrierCount = 1;
    shaderDi.pImageMemoryBarriers = &shaderImb;
    commandBuffer.pipelineBarrier2(shaderDi);
}

} // namespace muon::graphics
