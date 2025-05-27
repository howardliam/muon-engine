#pragma once

#include <SDL3/SDL_events.h>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Window;
    class Device;
    class DescriptorPool;
    class Image;

    class DebugUi {
    public:
        DebugUi(Window &window, Device &device);
        ~DebugUi();

        void pollEvents();

        void beginRendering(vk::CommandBuffer cmd);
        void endRendering(vk::CommandBuffer cmd);

        Image *getImage() const;

        void recreateSizedResources();

    private:
        Window &window;
        Device &device;

        bool showWindow{true};

        vk::Format format{vk::Format::eR8G8B8A8Unorm};
        vk::ImageLayout layout{vk::ImageLayout::eColorAttachmentOptimal};

        vk::DescriptorPool descriptorPool;
        vk::RenderPass renderPass;
        vk::PipelineCache cache;
        std::unique_ptr<Image> image;
        vk::Framebuffer framebuffer;

        void createResources();
        void createSizedResources();

        void initImGui();
    };

}
