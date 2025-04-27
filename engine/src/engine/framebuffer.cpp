#include "muon/engine/framebuffer.hpp"

#include <print>
#include <tuple>

namespace muon::engine {

    Framebuffer::Framebuffer(Device &device, const RenderPass &renderPass, vk::Extent2D extent) : device(device), extent(extent) {
        createImageView(renderPass.getImageFormat());
        createDepthImageView(renderPass.getDepthImageFormat());
        createFramebuffer(renderPass.getRenderPass());
    }

    Framebuffer::~Framebuffer() {
        device.getDevice().destroyFramebuffer(framebuffer, nullptr);

        device.getDevice().destroyImageView(depthImageView, nullptr);
        device.getAllocator().freeMemory(depthImageAllocation);
        device.getDevice().destroyImage(depthImage, nullptr);

        device.getDevice().destroyImageView(imageView, nullptr);
        device.getAllocator().freeMemory(imageAllocation);
        device.getDevice().destroyImage(image, nullptr);
    }

    vk::Framebuffer Framebuffer::getFramebuffer() const {
        return framebuffer;
    }

    vk::Extent2D Framebuffer::getExtent() const {
        return extent;
    }

    vk::Image Framebuffer::getImage() const {
        return image;
    }

    void Framebuffer::createImageView(vk::Format format) {
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
        imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.flags = vk::ImageCreateFlagBits{};

        device.createImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, vma::MemoryUsage::eGpuOnly, image, imageAllocation);

        vk::ImageViewCreateInfo createInfo{};
        createInfo.image = image;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = format;

        createInfo.components.r = vk::ComponentSwizzle::eR;
        createInfo.components.g = vk::ComponentSwizzle::eG;
        createInfo.components.b = vk::ComponentSwizzle::eB;
        createInfo.components.a = vk::ComponentSwizzle::eA;

        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        auto result = device.getDevice().createImageView(&createInfo, nullptr, &imageView);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create an image view");
        }
    }

    void Framebuffer::createDepthImageView(vk::Format format) {
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
        imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.flags = vk::ImageCreateFlagBits{};

        device.createImage(imageInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, vma::MemoryUsage::eGpuOnly, depthImage, depthImageAllocation);

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = depthImage;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = format;

        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = device.getDevice().createImageView(&viewInfo, nullptr, &depthImageView);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create a depth image view");
        }
    }

    void Framebuffer::createFramebuffer(vk::RenderPass renderPass) {
        std::array<vk::ImageView, 2> attachments = {imageView, depthImageView};

        vk::FramebufferCreateInfo createInfo{};
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.width = extent.width;
        createInfo.height = extent.height;
        createInfo.layers = 1;

        auto result = device.getDevice().createFramebuffer(&createInfo, nullptr, &framebuffer);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }

    Framebuffer2::Framebuffer2(
        Device &device,
        vk::RenderPass renderPass,
        const std::vector<vk::AttachmentDescription> &attachments,
        vk::Extent2D extent
    ) : device(device), extent(extent) {
        auto flagsFromLayout = [](vk::ImageLayout layout) -> std::tuple<vk::ImageUsageFlags, vk::AccessFlags, vk::PipelineStageFlags> {
            switch (layout) {
            case vk::ImageLayout::eColorAttachmentOptimal:
                return {
                    vk::ImageUsageFlagBits::eColorAttachment,
                    vk::AccessFlagBits::eColorAttachmentWrite,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput
                };

            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                return {
                    vk::ImageUsageFlagBits::eDepthStencilAttachment,
                    vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                    vk::PipelineStageFlagBits::eEarlyFragmentTests
                };

            default:
                return {{}, {}, {}};
            }
        };

        std::vector<vk::ImageView> imageViews{};
        for (const auto attachment : attachments) {
            auto [usageFlags, accessFlags, pipelineStageFlags] = flagsFromLayout(attachment.finalLayout);

            auto image = Image::Builder(device)
                .setExtent(extent)
                .setFormat(attachment.format)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(attachment.finalLayout)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(pipelineStageFlags)
                .buildUniquePtr();

            imageViews.push_back(image->getImageView());
            images.push_back(std::move(image));
        }

        vk::FramebufferCreateInfo createInfo{};
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
        createInfo.pAttachments = imageViews.data();
        createInfo.width = extent.width;
        createInfo.height = extent.height;
        createInfo.layers = 1;

        auto result = device.getDevice().createFramebuffer(&createInfo, nullptr, &framebuffer);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }

    Framebuffer2::~Framebuffer2() {
        device.getDevice().destroyFramebuffer(framebuffer, nullptr);
    }

}
