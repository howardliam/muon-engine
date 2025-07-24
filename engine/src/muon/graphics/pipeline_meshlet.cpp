#include "muon/graphics/pipeline_meshlet.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

PipelineMeshlet::PipelineMeshlet(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    // MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Meshlet, "must be meshlet pipeline config");
    // MU_CORE_ASSERT(spec.pipelineInfo.state.has_value(), "pipeline state must exist for meshlet pipeline");

    // const auto &state = *spec.pipelineInfo.state;
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

    MU_CORE_DEBUG("created meshlet pipeline");
}

PipelineMeshlet::~PipelineMeshlet() { MU_CORE_DEBUG("destroyed meshlet pipeline"); }

auto PipelineMeshlet::Bake(const vk::PipelineRenderingCreateInfo &renderingCi) -> void { CreatePipeline(renderingCi); }

auto PipelineMeshlet::Bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) -> void {
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout->Get(), 0, sets, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}

auto PipelineMeshlet::CreatePipeline(const vk::PipelineRenderingCreateInfo &renderingCi) -> void {
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

    graphicsPipelineCi.layout = m_layout->Get();
    graphicsPipelineCi.pNext = &renderingCi;
    graphicsPipelineCi.subpass = 0;

    graphicsPipelineCi.basePipelineIndex = -1;
    graphicsPipelineCi.basePipelineHandle = nullptr;

    auto createPipelineResult = m_context.GetDevice().createGraphicsPipeline(m_cache, graphicsPipelineCi);
    MU_CORE_ASSERT(createPipelineResult, "failed to create meshlet pipeline");

    m_pipeline = std::move(*createPipelineResult);
}

} // namespace muon::graphics
