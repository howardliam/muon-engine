#pragma once

#include "muon/engine/device.hpp"
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    class RenderPass {
    public:
        RenderPass(Device &device);
        RenderPass(Device &device, vk::Format imageFormat, vk::Format depthImageFormat);
        ~RenderPass();

        void begin(vk::CommandBuffer commandBuffer, vk::Framebuffer framebuffer, vk::Extent2D extent);
        void end(vk::CommandBuffer commandBuffer);

        [[nodiscard]] vk::RenderPass getRenderPass() const;
        [[nodiscard]] vk::Format getImageFormat() const;
        [[nodiscard]] vk::Format getDepthImageFormat() const;

    private:
        Device &device;

        vk::RenderPass renderPass;

        vk::Format imageFormat{vk::Format::eR8G8B8A8Unorm};
        vk::Format depthImageFormat{vk::Format::eD32Sfloat};

        void createRenderPass();
    };

    class RenderPass2 {
    public:
        class Builder;

        RenderPass2(
            Device &device,
            const vk::RenderPassCreateInfo &createInfo,
            const std::vector<vk::AttachmentDescription> &attachments
        );
        ~RenderPass2();

        void begin(vk::CommandBuffer commandBuffer, vk::Framebuffer framebuffer, vk::Extent2D extent);
        void end(vk::CommandBuffer commandBuffer);

        [[nodiscard]] const std::vector<vk::AttachmentDescription> &getAttachments() const;
        [[nodiscard]] vk::RenderPass getRenderPass() const;

    private:
        Device &device;

        vk::RenderPass renderPass;

        std::vector<vk::AttachmentDescription> attachments{};
    };

    class RenderPass2::Builder {
    public:
        Builder(Device &device);

        Builder &addColorAttachment(vk::Format format);

        Builder &addDepthStencilAttachment(vk::Format format);

        RenderPass2 build() const;

    private:
        Device &device;

        std::vector<vk::AttachmentDescription> colorAttachments{};
        std::vector<vk::AttachmentReference> colorAttachmentReferences{};

        bool depthPresent{false};
        vk::AttachmentDescription depthAttachment;
        vk::AttachmentReference depthAttachmentReference;
    };

}
