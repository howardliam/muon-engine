#include "muon/graphics/pipeline_meshlet.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

#include <vulkan/vulkan_core.h>

namespace muon::graphics {

PipelineMeshlet::PipelineMeshlet(const Spec &spec) : PipelineBase(*spec.context, spec.layout) {
    MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Meshlet, "must be meshlet pipeline config");
    MU_CORE_ASSERT(spec.pipelineInfo.state.has_value(), "pipeline state must exist for meshlet pipeline");

    const auto &state = *spec.pipelineInfo.state;
    m_state.viewportState = state.viewport.ToVk();
    m_state.rasterizationState = state.rasterization.ToVk();
    m_state.multisampleState = state.multisample.ToVk();
    const auto [colorBlendState, colorBlendAttachments] = state.colorBlend.ToVk();
    m_state.colorBlendAttachments = colorBlendAttachments;
    m_state.colorBlendState = colorBlendState;
    m_state.depthStencilState = state.depthStencil.ToVk();
    const auto [dynamicState, dynamicStateEnables] = state.dynamic.ToVk();
    m_state.dynamicStateEnables = dynamicStateEnables;
    m_state.dynamicState = dynamicState;

    const auto &shaders = spec.pipelineInfo.shaders;
    m_shaders.reserve(shaders.size());
    m_shaderStages.reserve(shaders.size());

    for (const auto &[stage, shaderInfo] : shaders) {
        auto shader = m_shaders.emplace_back(nullptr);
        CreateShaderModule(shaderInfo, shader);

        m_shaderStages.push_back(CreateShaderStageInfo(static_cast<VkShaderStageFlagBits>(stage), shader, shaderInfo.entryPoint));
    }

    MU_CORE_DEBUG("created meshlet pipeline");
}

PipelineMeshlet::~PipelineMeshlet() { MU_CORE_DEBUG("destroyed meshlet pipeline"); }

auto PipelineMeshlet::Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void {
    if (m_pipeline) {
        vkDestroyPipeline(m_context.GetDevice(), m_pipeline, nullptr);
    }

    CreatePipeline(renderingCreateInfo);
}

auto PipelineMeshlet::Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) -> void {
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout->Get(), 0, sets.size(), sets.data(), 0, nullptr);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

auto PipelineMeshlet::CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void {
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
    pipelineCreateInfo.pStages = m_shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pViewportState = &m_state.viewportState;
    pipelineCreateInfo.pRasterizationState = &m_state.rasterizationState;
    pipelineCreateInfo.pMultisampleState = &m_state.multisampleState;
    pipelineCreateInfo.pColorBlendState = &m_state.colorBlendState;
    pipelineCreateInfo.pDepthStencilState = &m_state.depthStencilState;
    pipelineCreateInfo.pDynamicState = &m_state.dynamicState;

    pipelineCreateInfo.layout = m_layout->Get();
    pipelineCreateInfo.pNext = &renderingCreateInfo;
    pipelineCreateInfo.subpass = 0;

    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.basePipelineHandle = nullptr;

    auto result = vkCreateGraphicsPipelines(m_context.GetDevice(), m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create meshlet pipeline");
}

} // namespace muon::graphics
