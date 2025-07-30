#include "muon/graphics/pipeline_graphics.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

PipelineGraphics::PipelineGraphics(const Spec &spec) : PipelineBase{spec.context, spec.layout}, m_state{spec.state} {
    core::debug("created graphics pipeline");
}

PipelineGraphics::~PipelineGraphics() { core::debug("destroyed graphics pipeline"); }

auto PipelineGraphics::bake(const vk::PipelineRenderingCreateInfo &renderingCreateInfo) -> void {
    createPipeline(renderingCreateInfo);
}

auto PipelineGraphics::bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) -> void {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout->get(), 0, sets, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}

auto PipelineGraphics::createPipeline(const vk::PipelineRenderingCreateInfo &renderingCreateInfo) -> void {
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCi;

    if (m_bindingDescription) {
        vertexInputStateCi.vertexBindingDescriptionCount = 1;
        vertexInputStateCi.pVertexBindingDescriptions = &m_bindingDescription.value();
        vertexInputStateCi.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
        vertexInputStateCi.pVertexAttributeDescriptions = m_attributeDescriptions.data();
    } else {
        vertexInputStateCi.vertexBindingDescriptionCount = 0;
        vertexInputStateCi.pVertexBindingDescriptions = nullptr;
        vertexInputStateCi.vertexAttributeDescriptionCount = 0;
        vertexInputStateCi.pVertexAttributeDescriptions = nullptr;
    }

    vk::GraphicsPipelineCreateInfo graphicsPipelineCi;
    graphicsPipelineCi.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    graphicsPipelineCi.pStages = m_shaderStages.data();
    graphicsPipelineCi.pVertexInputState = &vertexInputStateCi;
    graphicsPipelineCi.pInputAssemblyState = &m_state.inputAssemblyState;
    graphicsPipelineCi.pViewportState = &m_state.viewportState;
    graphicsPipelineCi.pRasterizationState = &m_state.rasterizationState;
    graphicsPipelineCi.pMultisampleState = &m_state.multisampleState;
    graphicsPipelineCi.pColorBlendState = &m_state.colorBlendState;
    graphicsPipelineCi.pDepthStencilState = &m_state.depthStencilState;
    graphicsPipelineCi.pDynamicState = &m_state.dynamicState;

    graphicsPipelineCi.layout = m_layout->get();
    graphicsPipelineCi.pNext = &renderingCreateInfo;
    graphicsPipelineCi.subpass = 0;

    graphicsPipelineCi.basePipelineIndex = -1;
    graphicsPipelineCi.basePipelineHandle = nullptr;

    auto createPipelineResult = m_context.getDevice().createGraphicsPipeline(m_cache, graphicsPipelineCi);
    core::expect(createPipelineResult, "failed to create graphics pipeline");

    m_pipeline = std::move(*createPipelineResult);
}

} // namespace muon::graphics
