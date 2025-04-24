#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/pipeline/compute.hpp"
#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    class ComputeSystem {
    public:
        ComputeSystem(Device &device, std::vector<vk::DescriptorSetLayout> setLayouts);
        ~ComputeSystem();

    protected:
        Device &device;

        std::unique_ptr<ComputePipeline> pipeline;
        vk::PipelineLayout pipelineLayout;

        /**
         * @brief   creates a pipeline layout for the compute system.
         *
         * @param   setLayouts  the descriptor set layouts to use.
         */
        void createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts);

        /**
         * @brief   creates a pipeline for the compute system.
         *
         * Use your implementation to configure push constants.
         */
        virtual void createPipeline() = 0;
    };

}
