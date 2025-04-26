#pragma once

#include "muon/engine/device.hpp"

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

}
