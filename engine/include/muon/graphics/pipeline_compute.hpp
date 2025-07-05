#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/pipeline_info.hpp"
#include <glm/vec3.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineComputeSpecification {
        const DeviceContext *device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        schematic::PipelineInfo pipelineInfo{};
    };

    class PipelineCompute : PipelineBase {
    public:
        PipelineCompute(const PipelineComputeSpecification &spec);
        ~PipelineCompute();

        auto Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) const -> void;
        auto Dispatch(VkCommandBuffer cmd, const glm::uvec3 &groupCount) const -> void;

    public:
        [[nodiscard]] auto Get() const -> VkPipeline;

    private:
        auto CreatePipeline(const VkPipelineShaderStageCreateInfo &stageInfo) -> void;
    };

}
