#pragma once

#include "muon/engine/device.hpp"
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    class RenderPass {
    public:
        class Builder;

        RenderPass(
            Device &device,
            const vk::RenderPassCreateInfo &createInfo,
            const std::vector<vk::AttachmentDescription> &attachments
        );
        ~RenderPass();

        void begin(vk::CommandBuffer commandBuffer, vk::Framebuffer framebuffer, vk::Extent2D extent);
        void end(vk::CommandBuffer commandBuffer);

        [[nodiscard]] const std::vector<vk::AttachmentDescription> &getAttachments() const;
        [[nodiscard]] vk::RenderPass getRenderPass() const;

    private:
        Device &device;

        vk::RenderPass renderPass;

        std::vector<vk::AttachmentDescription> attachments{};
    };

    class RenderPass::Builder {
    public:
        Builder(Device &device);

        Builder &addColorAttachment(vk::Format format);

        Builder &addDepthStencilAttachment(vk::Format format);

        RenderPass build() const;

    private:
        Device &device;

        std::vector<vk::AttachmentDescription> colorAttachments{};
        std::vector<vk::AttachmentReference> colorAttachmentRefs{};

        bool hasDepthStencil{false};
        vk::AttachmentDescription depthStencilAttachment;
        vk::AttachmentReference depthStencilAttachmentRef;
    };

}
