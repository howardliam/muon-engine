#include "muon/graphics/texture.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/utils/pretty_print.hpp"
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

namespace muon::graphics {

    Texture::Texture(const Spec &spec) : m_device(*spec.device), m_extent(spec.extent), m_format(spec.format) {
        CreateImage();
        CreateImageView();
        CreateSampler();

        m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_descriptorInfo.imageView = m_imageView;
        m_descriptorInfo.sampler = m_sampler;

        auto cmd = spec.cmd;
        bool noCommandBuffer = spec.cmd == nullptr;

        if (noCommandBuffer) { cmd = m_device.GetTransferQueue().BeginCommands(); }
        UploadData(cmd, spec.textureData, spec.pixelSize);
        if (noCommandBuffer) { m_device.GetTransferQueue().EndCommands(cmd); }

        MU_CORE_DEBUG("created texture with dimensions: {}x{}, and size: {}", m_extent.width, m_extent.height, pp::PrintBytes(m_bytes));
    }

    Texture::~Texture() {
        vkDestroySampler(m_device.GetDevice(), m_sampler, nullptr);
        vkDestroyImageView(m_device.GetDevice(), m_imageView, nullptr);
        vmaDestroyImage(m_device.GetAllocator(), m_image, m_allocation);

        MU_CORE_DEBUG("destroyed texture");
    }

    auto Texture::Get() const -> VkImage {
        return m_image;
    }

    auto Texture::GetView() const -> VkImageView {
        return m_imageView;
    }

    auto Texture::GetSampler() const -> VkSampler {
        return m_sampler;
    }

    auto Texture::GetDescriptorInfo() const -> const VkDescriptorImageInfo & {
        return m_descriptorInfo;
    }

    auto Texture::CreateImage() -> void {
        VkImageCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.extent = { m_extent.width, m_extent.height, 1 };
        createInfo.mipLevels = 1;
        createInfo.arrayLayers = 1;
        createInfo.format = m_format;
        createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.flags = 0;

        auto result = vkCreateImage(m_device.GetDevice(), &createInfo, nullptr, &m_image);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create texture image");

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device.GetDevice(), m_image, &memoryRequirements);

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VmaAllocationInfo allocInfo{};
        result = vmaAllocateMemory(m_device.GetAllocator(), &memoryRequirements, &allocCreateInfo, &m_allocation, &allocInfo);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate texture image memory");

        result = vmaBindImageMemory(m_device.GetAllocator(), m_allocation, m_image);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to bind texture image memory");

        m_bytes = allocInfo.size;
    }

    auto Texture::CreateImageView() -> void {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(m_device.GetDevice(), &createInfo, nullptr, &m_imageView);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create texture image view");
    }

    auto Texture::CreateSampler() -> void {
        VkSamplerCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.compareEnable = false;
        createInfo.compareOp = VK_COMPARE_OP_NEVER;
        createInfo.mipLodBias = 0.0;
        createInfo.minLod = 0.0;
        createInfo.maxLod = 0.0;
        createInfo.anisotropyEnable = false;
        createInfo.maxAnisotropy = 4.0;
        createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        auto result = vkCreateSampler(m_device.GetDevice(), &createInfo, nullptr, &m_sampler);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create texture samper");
    }

    auto Texture::UploadData(VkCommandBuffer cmd, const std::vector<uint8_t> &textureData, uint32_t pixelSize) -> void {
        VkImageMemoryBarrier2 copyBarrier{};
        copyBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        copyBarrier.image = m_image;
        copyBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyBarrier.subresourceRange.baseMipLevel = 0;
        copyBarrier.subresourceRange.levelCount = 1;
        copyBarrier.subresourceRange.baseArrayLayer = 0;
        copyBarrier.subresourceRange.layerCount = 1;

        copyBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copyBarrier.srcAccessMask = VK_ACCESS_2_NONE;
        copyBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;

        copyBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        copyBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copyBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

        VkDependencyInfo copyDepInfo{};
        copyDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        copyDepInfo.dependencyFlags = 0;
        copyDepInfo.imageMemoryBarrierCount = 1;
        copyDepInfo.pImageMemoryBarriers = &copyBarrier;
        vkCmdPipelineBarrier2(cmd, &copyDepInfo);

        Buffer::Spec stagingBufferSpec{};
        stagingBufferSpec.device = &m_device;
        stagingBufferSpec.instanceSize = pixelSize;
        stagingBufferSpec.instanceCount = textureData.size() / pixelSize;
        stagingBufferSpec.usageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferSpec.memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
        Buffer stagingBuffer{stagingBufferSpec};

        auto result = stagingBuffer.Map();
        MU_CORE_ASSERT(result == VK_SUCCESS, "faild to map texture staging buffer");

        stagingBuffer.Write(textureData.data());

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { m_extent.width, m_extent.height, 1 };

        vkCmdCopyBufferToImage(cmd, stagingBuffer.Get(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier2 finalBarrier{};
        finalBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        finalBarrier.image = m_image;
        finalBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        finalBarrier.subresourceRange.baseMipLevel = 0;
        finalBarrier.subresourceRange.levelCount = 1;
        finalBarrier.subresourceRange.baseArrayLayer = 0;
        finalBarrier.subresourceRange.layerCount = 1;

        finalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        finalBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        finalBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;

        finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        finalBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        finalBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalBarrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

        VkDependencyInfo finalDepInfo{};
        finalDepInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        finalDepInfo.dependencyFlags = 0;
        finalDepInfo.imageMemoryBarrierCount = 1;
        finalDepInfo.pImageMemoryBarriers = &finalBarrier;
        vkCmdPipelineBarrier2(cmd, &finalDepInfo);
    }

}
