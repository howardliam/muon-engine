#pragma once

#include "muon/graphics/pipeline_base.hpp"
#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    class PipelineCompute : PipelineBase {
    public:
        PipelineCompute(const PipelineSpecification &spec);
        ~PipelineCompute();

        auto Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) const -> void;
        auto Dispatch(VkCommandBuffer cmd, const glm::uvec3 &groupCount) const -> void;

    public:
        [[nodiscard]] auto Get() const -> VkPipeline;

    private:
        auto CreatePipeline(const VkPipelineShaderStageCreateInfo &stageInfo) -> void;
    };

}
