#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class Window;
    class Device;
    class DescriptorPool;
    class Image;

    class DebugUi {
    public:
        DebugUi(Window &window, Device &device);
        ~DebugUi();

        void beginRendering(vk::CommandBuffer cmd);
        void endRendering(vk::CommandBuffer cmd);

        Image *getImage() const;

    private:
        Window &window;
        Device &device;

        std::unique_ptr<DescriptorPool> descriptorPool;
        vk::RenderPass renderPass;
        std::unique_ptr<Image> image;
        vk::Framebuffer framebuffer;

        void createResources();

        void initImGui();
    };

}
