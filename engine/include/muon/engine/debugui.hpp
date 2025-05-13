#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class Window;
    class Device;
    class DescriptorPool;

    class DebugUi {
    public:
        DebugUi(Window &window, Device &device);
        ~DebugUi();

    private:
        Window &window;
        Device &device;

        std::unique_ptr<DescriptorPool> descriptorPool;
        vk::RenderPass renderPass;

        void createResources();
    };

}
