#include "muon/graphics/pipeline_compute.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"

namespace muon::graphics {

PipelineCompute::PipelineCompute(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    core::debug("created compute pipeline");
}

PipelineCompute::~PipelineCompute() { core::debug("destroyed compute pipeline"); }

auto PipelineCompute::bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) const -> void {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_layout->get(), 0, sets, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);
}

auto PipelineCompute::dispatch(vk::raii::CommandBuffer &commandBuffer, const glm::uvec3 &groupCount) const -> void {
    commandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
}

auto PipelineCompute::createPipeline(const vk::PipelineShaderStageCreateInfo &stageInfo) -> void {
    vk::ComputePipelineCreateInfo computePipelineCi;
    computePipelineCi.stage = stageInfo;
    computePipelineCi.layout = m_layout->get();
    computePipelineCi.basePipelineIndex = -1;
    computePipelineCi.basePipelineHandle = nullptr;

    auto createPipelineResult = m_context.getDevice().createComputePipeline(m_cache, computePipelineCi);
    core::expect(createPipelineResult, "failed to create compute pipeline");

    m_pipeline = std::move(*createPipelineResult);
}

} // namespace muon::graphics
