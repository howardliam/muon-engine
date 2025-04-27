#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/image.hpp"
#include <vulkan/vulkan_handles.hpp>
#include <memory>

namespace muon::engine {

    class Framebuffer {
    public:
        Framebuffer(
            Device &device,
            vk::RenderPass renderPass,
            const std::vector<vk::AttachmentDescription> &attachments,
            vk::Extent2D extent
        );
        ~Framebuffer();

        [[nodiscard]] vk::Framebuffer getFramebuffer() const;
        [[nodiscard]] vk::Extent2D getExtent() const;
        [[nodiscard]] std::optional<Image *> getImage(size_t index) const;

    private:
        Device &device;

        vk::Framebuffer framebuffer;
        vk::Extent2D extent;

        std::vector<std::unique_ptr<Image>> images{};
    };

}
