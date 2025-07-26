#include "muon/graphics/pipeline_meshlet.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

PipelineMeshlet::PipelineMeshlet(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    core::debug("created meshlet pipeline");
}

PipelineMeshlet::~PipelineMeshlet() { core::debug("destroyed meshlet pipeline"); }

auto PipelineMeshlet::bake(const vk::PipelineRenderingCreateInfo &renderingCi) -> void { createPipeline(renderingCi); }

auto PipelineMeshlet::bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) -> void {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout->get(), 0, sets, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}

auto PipelineMeshlet::createPipeline(const vk::PipelineRenderingCreateInfo &renderingCi) -> void {
    vk::GraphicsPipelineCreateInfo graphicsPipelineCi;
    graphicsPipelineCi.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    graphicsPipelineCi.pStages = m_shaderStages.data();
    graphicsPipelineCi.pVertexInputState = nullptr;
    graphicsPipelineCi.pViewportState = &m_state.viewportState;
    graphicsPipelineCi.pRasterizationState = &m_state.rasterizationState;
    graphicsPipelineCi.pMultisampleState = &m_state.multisampleState;
    graphicsPipelineCi.pColorBlendState = &m_state.colorBlendState;
    graphicsPipelineCi.pDepthStencilState = &m_state.depthStencilState;
    graphicsPipelineCi.pDynamicState = &m_state.dynamicState;

    graphicsPipelineCi.layout = m_layout->get();
    graphicsPipelineCi.pNext = &renderingCi;
    graphicsPipelineCi.subpass = 0;

    graphicsPipelineCi.basePipelineIndex = -1;
    graphicsPipelineCi.basePipelineHandle = nullptr;

    auto createPipelineResult = m_context.getDevice().createGraphicsPipeline(m_cache, graphicsPipelineCi);
    core::expect(createPipelineResult, "failed to create meshlet pipeline");

    m_pipeline = std::move(*createPipelineResult);
}

} // namespace muon::graphics
