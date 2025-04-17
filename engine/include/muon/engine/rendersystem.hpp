#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/pipeline.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class RenderSystem {
    public:
        RenderSystem(Device &device, std::vector<vk::DescriptorSetLayout> setLayouts);
        ~RenderSystem();

    protected:
        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipelineLayout;

        /**
         * @brief   creates a pipeline layout for the render system.
         *
         * @param   setLayouts  the descriptor set layouts to use.
         */
        void createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts);

        /**
         * @brief   creates a pipeline for the render system.
         *
         * Use your implementation to configure push constants.
         *
         * @param   renderPass  the render pass to use.
         */
        virtual void createPipeline(vk::RenderPass renderPass) = 0;
    };

}
