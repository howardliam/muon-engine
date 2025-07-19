#include "muon/graphics/pipeline_compute.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

PipelineCompute::PipelineCompute(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Compute, "must be compute pipeline config");

    const auto &shaderInfo = spec.pipelineInfo.shaders.find(VK_SHADER_STAGE_COMPUTE_BIT)->second;
    auto shader = m_shaders.emplace_back(nullptr);
    CreateShaderModule(shaderInfo, shader);
    const auto stageInfo = CreateShaderStageInfo(VK_SHADER_STAGE_COMPUTE_BIT, shader, shaderInfo.entryPoint);
    CreatePipeline(stageInfo);

    MU_CORE_DEBUG("created compute pipeline");
}

PipelineCompute::~PipelineCompute() { MU_CORE_DEBUG("destroyed compute pipeline"); }

auto PipelineCompute::Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) const -> void {
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_layout->Get(), 0, sets.size(), sets.data(), 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
}

auto PipelineCompute::Dispatch(VkCommandBuffer cmd, const glm::uvec3 &groupCount) const -> void {
    vkCmdDispatch(cmd, groupCount.x, groupCount.y, groupCount.z);
}

auto PipelineCompute::Get() const -> VkPipeline { return m_pipeline; }

auto PipelineCompute::CreatePipeline(const VkPipelineShaderStageCreateInfo &stageInfo) -> void {
    VkComputePipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.stage = stageInfo;
    pipelineCreateInfo.layout = m_layout->Get();
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = nullptr;

    auto result = vkCreateComputePipelines(m_context.GetDevice(), m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create compute pipeline");
}

} // namespace muon::graphics
