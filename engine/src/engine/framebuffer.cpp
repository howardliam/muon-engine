#include "muon/engine/framebuffer.hpp"

#include <optional>
#include <print>
#include <tuple>

namespace muon::engine {

    Framebuffer::Framebuffer(
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

    Framebuffer::~Framebuffer() {
        device.getDevice().destroyFramebuffer(framebuffer, nullptr);
    }

    vk::Framebuffer Framebuffer::getFramebuffer() const {
        return framebuffer;
    }

    vk::Extent2D Framebuffer::getExtent() const {
        return extent;
    }

    std::optional<Image *> Framebuffer::getImage(size_t index) const {
        if (index >= images.size()) {
            return {};
        }

        return images[index].get();
    }

}
