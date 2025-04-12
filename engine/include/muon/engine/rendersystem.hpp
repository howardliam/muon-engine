#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/pipeline.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class IRenderSystem {
    public:
        IRenderSystem(Device &device/*, vk::DescriptorSetLayout layouts... */);

        virtual void renderModel(/* FrameInfo frameInfo, Model &model */) = 0;

    private:
        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipelineLayout;

        void createPipelineLayout(/*vk::DescriptorSetLayout layouts... */);
        void createPipeline(vk::RenderPass renderPass);
    };

}
