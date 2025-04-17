#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/renderpass.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class Framebuffer {
    public:
        Framebuffer(Device &device, const RenderPass &renderPass, vk::Extent2D extent);
        ~Framebuffer();

        [[nodiscard]] vk::Framebuffer getFramebuffer() const;
        [[nodiscard]] vk::Extent2D getExtent() const;
        [[nodiscard]] vk::Image getImage() const;

    private:
        Device &device;

        vk::Framebuffer framebuffer;
        vk::Extent2D extent;

        vk::Image image;
        vk::ImageView imageView;
        vma::Allocation imageAllocation;

        vk::Image depthImage;
        vk::ImageView depthImageView;
        vma::Allocation depthImageAllocation;

        void createImageView(vk::Format format);
        void createDepthImageView(vk::Format format);
        void createFramebuffer(vk::RenderPass renderPass);
    };

}
