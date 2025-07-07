#include "muon/graphics/image.hpp"

#include "muon/core/assert.hpp"
#include "muon/utils/pretty_print.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    Image::Image(const ImageSpecification &spec) : m_device(*spec.device), m_extent(spec.extent), m_format(spec.format), m_layout(spec.layout), m_usageFlags(spec.usageFlags), m_accessFlags(spec.accessFlags), m_stageFlags(spec.stageFlags) {
        CreateImage();
        CreateImageView();

        auto cmd = spec.cmd;
        bool noCommandBuffer = spec.cmd == nullptr;

        if (noCommandBuffer) { cmd = m_device.GetTransferQueue().BeginCommands(); }
        TransitionLayout(cmd);
        if (noCommandBuffer) { m_device.GetTransferQueue().EndCommands(cmd); }

        MU_CORE_DEBUG("created image with dimensions: {}x{}, and size: {}", m_extent.width, m_extent.height, pp::PrintBytes(m_bytes));
    }

    Image::~Image() {
        vkDestroyImageView(m_device.GetDevice(), m_imageView, nullptr);
        vmaDestroyImage(m_device.GetAllocator(), m_image, m_allocation);

        MU_CORE_DEBUG("destroyed image");
    }

    auto Image::GetExtent() const -> VkExtent2D {
        return m_extent;
    }

    auto Image::GetFormat() const -> VkFormat {
        return m_format;
    }

    auto Image::GetLayout() const -> VkImageLayout {
        return m_layout;
    }

    auto Image::GetUsageFlags() const -> VkImageUsageFlags {
        return m_usageFlags;
    }

    auto Image::GetAccessFlags() const -> VkAccessFlags2 {
        return m_accessFlags;
    }

    auto Image::GetStageFlags() const -> VkPipelineStageFlags2 {
        return m_stageFlags;
    }

    auto Image::Get() const -> VkImage {
        return m_image;
    }

    auto Image::GetView() const -> VkImageView {
        return m_imageView;
    }

    auto Image::GetDescriptorInfo() const -> const VkDescriptorImageInfo & {
        return m_descriptorInfo;
    }

    auto Image::CreateImage() -> void {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags = 0;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_format;
        imageInfo.extent = { m_extent.width, m_extent.height, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = m_usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        auto result = vkCreateImage(m_device.GetDevice(), &imageInfo, nullptr, &m_image);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create image");

        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(m_device.GetDevice(), m_image, &memoryRequirements);

        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        VmaAllocationInfo allocInfo{};
        result = vmaAllocateMemory(m_device.GetAllocator(), &memoryRequirements, &allocCreateInfo, &m_allocation, &allocInfo);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate image memory");

        result = vmaBindImageMemory(m_device.GetAllocator(), m_allocation, m_image);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to bind image memory");

        m_bytes = allocInfo.size;
    }

    auto Image::CreateImageView() -> void {
        m_aspectMask = [](VkFormat format) -> VkImageAspectFlags {
            switch (format) {
                case VK_FORMAT_UNDEFINED: {
                    return VK_IMAGE_ASPECT_NONE;
                }

                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D32_SFLOAT:
                case VK_FORMAT_X8_D24_UNORM_PACK32: {
                    return VK_IMAGE_ASPECT_DEPTH_BIT;
                }

                case VK_FORMAT_S8_UINT: {
                    return VK_IMAGE_ASPECT_STENCIL_BIT;
                }

                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT: {
                    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                }

                default: {
                    return VK_IMAGE_ASPECT_COLOR_BIT;
                }
            }
        }(m_format);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = m_aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(m_device.GetDevice(), &viewInfo, nullptr, &m_imageView);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create image view");

        m_descriptorInfo.imageLayout = m_layout;
        m_descriptorInfo.imageView = m_imageView;
        m_descriptorInfo.sampler = nullptr;
    }

    auto Image::TransitionLayout(VkCommandBuffer cmd) -> void {
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = m_aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;

        barrier.newLayout = m_layout;
        barrier.dstAccessMask = m_accessFlags;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstStageMask = m_stageFlags;

        VkDependencyInfo depInfo{};
        depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        depInfo.dependencyFlags = 0;
        depInfo.imageMemoryBarrierCount = 1;
        depInfo.pImageMemoryBarriers = &barrier;
        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

}
