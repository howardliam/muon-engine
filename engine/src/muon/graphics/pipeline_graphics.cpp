#include "muon/graphics/pipeline_graphics.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

PipelineGraphics::PipelineGraphics(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    // MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Graphics, "must be graphics pipeline config");
    // MU_CORE_ASSERT(spec.pipelineInfo.state.has_value(), "pipeline state must exist for graphics pipeline");
    // MU_CORE_ASSERT(
    //     spec.pipelineInfo.state->inputAssembly.has_value(), "pipeline state must have input assembly info for graphics
    //     pipeline"
    // );

    // const auto &state = *spec.pipelineInfo.state;
    // m_state.inputAssemblyState = state.inputAssembly->ToVk();
    // m_state.viewportState = state.viewport.ToVk();
    // m_state.rasterizationState = state.rasterization.ToVk();
    // m_state.multisampleState = state.multisample.ToVk();
    // const auto [colorBlendState, colorBlendAttachments] = state.colorBlend.ToVk();
    // m_state.colorBlendAttachments = colorBlendAttachments;
    // m_state.colorBlendState = colorBlendState;
    // m_state.depthStencilState = state.depthStencil.ToVk();
    // const auto [dynamicState, dynamicStateEnables] = state.dynamic.ToVk();
    // m_state.dynamicStateEnables = dynamicStateEnables;
    // m_state.dynamicState = dynamicState;

    // const auto &shaders = spec.pipelineInfo.shaders;
    // m_shaders.reserve(shaders.size());
    // m_shaderStages.reserve(shaders.size());

    // for (const auto &[stage, shaderInfo] : shaders) {
    //     auto shader = m_shaders.emplace_back(nullptr);
    //     CreateShaderModule(shaderInfo, shader);

    //     m_shaderStages.push_back(CreateShaderStageInfo(static_cast<VkShaderStageFlagBits>(stage), shader,
    //     shaderInfo.entryPoint));
    // }

    MU_CORE_DEBUG("created graphics pipeline");
}

PipelineGraphics::~PipelineGraphics() { MU_CORE_DEBUG("destroyed graphics pipeline"); }

auto PipelineGraphics::Bake(const vk::PipelineRenderingCreateInfo &renderingCreateInfo) -> void {
    CreatePipeline(renderingCreateInfo);
}

auto PipelineGraphics::Bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) -> void {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout->Get(), 0, sets, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}

auto PipelineGraphics::CreatePipeline(const vk::PipelineRenderingCreateInfo &renderingCreateInfo) -> void {
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

    graphicsPipelineCi.layout = m_layout->Get();
    graphicsPipelineCi.pNext = &renderingCreateInfo;
    graphicsPipelineCi.subpass = 0;

    graphicsPipelineCi.basePipelineIndex = -1;
    graphicsPipelineCi.basePipelineHandle = nullptr;

    auto createPipelineResult = m_context.GetDevice().createGraphicsPipeline(m_cache, graphicsPipelineCi);
    MU_CORE_ASSERT(createPipelineResult, "failed to create graphics pipeline");

    m_pipeline = std::move(*createPipelineResult);
}

} // namespace muon::graphics
