#pragma once

#include "muon/graphics/context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <glm/vec3.hpp>
#include <memory>

namespace muon::graphics {

class PipelineCompute : PipelineBase {
public:
    struct Spec {
        const Context *context{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
    };

public:
    PipelineCompute(const Spec &spec);
    ~PipelineCompute();

    auto Bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) const -> void;
    auto Dispatch(vk::raii::CommandBuffer &commandBuffer, const glm::uvec3 &groupCount) const -> void;

private:
    auto CreatePipeline(const vk::PipelineShaderStageCreateInfo &stageInfo) -> void;
};

} // namespace muon::graphics
