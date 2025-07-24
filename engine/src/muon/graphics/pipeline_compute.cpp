#include "muon/graphics/pipeline_compute.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"

namespace muon::graphics {

PipelineCompute::PipelineCompute(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    // MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Compute, "must be compute pipeline config");

    // const auto &shaderInfo = spec.pipelineInfo.shaders.find(VK_SHADER_STAGE_COMPUTE_BIT)->second;
    // auto shader = m_shaders.emplace_back(nullptr);
    // CreateShaderModule(shaderInfo, shader);
    // const auto stageInfo = CreateShaderStageInfo(VK_SHADER_STAGE_COMPUTE_BIT, shader, shaderInfo.entryPoint);
    // CreatePipeline(stageInfo);

    MU_CORE_DEBUG("created compute pipeline");
}

PipelineCompute::~PipelineCompute() { MU_CORE_DEBUG("destroyed compute pipeline"); }

auto PipelineCompute::Bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) const -> void {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_layout->Get(), 0, sets, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);
}

auto PipelineCompute::Dispatch(vk::raii::CommandBuffer &commandBuffer, const glm::uvec3 &groupCount) const -> void {
    commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
}

auto PipelineCompute::CreatePipeline(const vk::PipelineShaderStageCreateInfo &stageInfo) -> void {
    vk::ComputePipelineCreateInfo computePipelineCi;
    computePipelineCi.stage = stageInfo;
    computePipelineCi.layout = m_layout->Get();
    computePipelineCi.basePipelineIndex = -1;
    computePipelineCi.basePipelineHandle = nullptr;

    auto createPipelineResult = m_context.GetDevice().createComputePipeline(m_cache, computePipelineCi);
    MU_CORE_ASSERT(createPipelineResult, "failed to create compute pipeline");

    m_pipeline = std::move(*createPipelineResult);
}

} // namespace muon::graphics
