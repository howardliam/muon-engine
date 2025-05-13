#pragma once

#include "muon/engine/pipeline/compute.hpp"
#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/vec3.hpp>

namespace muon::engine {

    class Device;

    class ComputeSystem {
    public:
        ComputeSystem(
            Device &device,
            std::vector<vk::DescriptorSetLayout> setLayouts,
            std::vector<vk::PushConstantRange> pushConstants
        );
        ~ComputeSystem();

        virtual void dispatch(
            vk::CommandBuffer commandBuffer,
            const std::vector<vk::DescriptorSet> &sets,
            vk::Extent2D windowExtent,
            const glm::uvec3 &workgroupSize
        ) = 0;

    protected:
        Device &device;

        std::unique_ptr<ComputePipeline> pipeline;
        vk::PipelineLayout pipelineLayout;

        glm::uvec3 calculateDispatchSize(vk::Extent2D windowExtent, const glm::uvec3 &workgroupSize);

        /**
         * @brief   creates a pipeline layout for the compute system.
         *
         * @param   setLayouts  the descriptor set layouts to use.
         */
        void createPipelineLayout(
            std::vector<vk::DescriptorSetLayout> setLayouts,
            std::vector<vk::PushConstantRange> pushConstants
        );

        /**
         * @brief   creates a pipeline for the compute system.
         *
         * Use your implementation to configure push constants.
         */
        virtual void createPipeline() = 0;
    };

}
