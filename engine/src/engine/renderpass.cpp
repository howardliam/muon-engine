#include "muon/engine/renderpass.hpp"
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    RenderPass::RenderPass(
        Device &device,
        const vk::RenderPassCreateInfo &createInfo,
        const std::vector<vk::AttachmentDescription> &attachments
    ) : device(device), attachments(attachments) {
        auto result = device.getDevice().createRenderPass(&createInfo, nullptr, &renderPass);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create render pass");
        }
    }

    RenderPass::~RenderPass() {
        device.getDevice().destroyRenderPass(renderPass, nullptr);
    }

    void RenderPass::begin(vk::CommandBuffer commandBuffer, vk::Framebuffer framebuffer, vk::Extent2D extent) {
        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffer;

        renderPassInfo.renderArea.offset.x = 0;
        renderPassInfo.renderArea.offset.y = 0;
        renderPassInfo.renderArea.extent = extent;

        std::array<vk::ClearValue, 2> clearValues;
        clearValues[0].color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = extent;

        commandBuffer.setViewport(0, 1, &viewport);
        commandBuffer.setScissor(0, 1, &scissor);
    }

    void RenderPass::end(vk::CommandBuffer commandBuffer) {
        commandBuffer.endRenderPass();
    }

    const std::vector<vk::AttachmentDescription> &RenderPass::getAttachments() const {
        return attachments;
    }

    vk::RenderPass RenderPass::getRenderPass() const {
        return renderPass;
    }

    RenderPass::Builder::Builder(Device &device) : device(device) {}

    RenderPass::Builder &RenderPass::Builder::addColorAttachment(vk::Format format) {
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format = format;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        colorAttachments.push_back(colorAttachment);
        colorAttachmentRefs.push_back(colorAttachmentRef);

        return *this;
    }

    RenderPass::Builder &RenderPass::Builder::addDepthStencilAttachment(vk::Format format) {
        vk::AttachmentDescription depthStencilAttachment{};
        depthStencilAttachment.format = format;
        depthStencilAttachment.samples = vk::SampleCountFlagBits::e1;
        depthStencilAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        depthStencilAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
        depthStencilAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
        depthStencilAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        depthStencilAttachment.initialLayout = vk::ImageLayout::eUndefined;
        depthStencilAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        vk::AttachmentReference depthStencilAttachmentRef{};
        depthStencilAttachmentRef.attachment = 1;
        depthStencilAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

        this->depthStencilAttachment = depthStencilAttachment;
        this->depthStencilAttachmentRef = depthStencilAttachmentRef;
        hasDepthStencil = true;

        return *this;
    }

    RenderPass RenderPass::Builder::build() const {
        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

        if (colorAttachmentRefs.size() > 0) {
            subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRefs.size());
            subpass.pColorAttachments = colorAttachmentRefs.data();
        } else {
            subpass.colorAttachmentCount = 0;
            subpass.pColorAttachments = nullptr;
        }

        if (hasDepthStencil) {
            subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;
        } else {
            subpass.pDepthStencilAttachment = nullptr;
        }

        vk::SubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = vk::SubpassExternal;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcAccessMask = vk::AccessFlagBits{};
        subpassDependency.dstAccessMask =
            vk::AccessFlagBits::eColorAttachmentWrite |
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        subpassDependency.srcStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests;
        subpassDependency.dstStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput |
            vk::PipelineStageFlagBits::eEarlyFragmentTests;

        std::vector<vk::AttachmentDescription> attachments{};

        if (colorAttachments.size() > 0) {
            attachments.insert(attachments.end(), colorAttachments.begin(), colorAttachments.end());
        }

        if (hasDepthStencil) {
            attachments.push_back(depthStencilAttachment);
        }

        vk::RenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        return RenderPass(device, renderPassCreateInfo, attachments);
    }
}
