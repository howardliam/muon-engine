#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/pipeline.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class RenderSystem {
    public:
        RenderSystem(Device &device, std::vector<vk::DescriptorSetLayout> setLayouts, vk::RenderPass renderPass);
        ~RenderSystem();

        virtual void renderModel(/* FrameInfo frameInfo, Model &model */) = 0;

    private:
        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipelineLayout;

        void createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts);
        void createPipeline(vk::RenderPass renderPass);
    };

}
