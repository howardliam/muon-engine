#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/pipeline/graphics.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class GraphicsSystem {
    public:
        GraphicsSystem(
            Device &device,
            std::vector<vk::DescriptorSetLayout> setLayouts,
            std::vector<vk::PushConstantRange> pushConstants
        );
        ~GraphicsSystem();

        /**
         * @brief   bakes the render system's pipeline.
         *
         * @param   renderPass  the render pass to use.
         */
        void bake(const vk::PipelineRenderingCreateInfo &renderingInfo);

    protected:
        Device &device;

        std::unique_ptr<GraphicsPipeline> pipeline;
        vk::PipelineLayout pipelineLayout;

        /**
         * @brief   creates a pipeline layout for the render system.
         *
         * @param   setLayouts  the descriptor set layouts to use.
         */
        void createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts, std::vector<vk::PushConstantRange> pushConstants);

        /**
         * @brief   creates a pipeline for the render system.
         *
         * Use your implementation to configure push constants.
         *
         * @param   renderPass  the render pass to use.
         */
        virtual void createPipeline(const vk::PipelineRenderingCreateInfo &renderingInfo) = 0;
    };

}
