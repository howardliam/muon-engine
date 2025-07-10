#include "muon/graphics/pipeline_graphics.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    PipelineGraphics::PipelineGraphics(const Spec &spec) : PipelineBase(*spec.device, spec.layout) {
        MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Graphics, "must be graphics pipeline config");
        MU_CORE_ASSERT(spec.pipelineInfo.state.has_value(), "pipeline state must exist for graphics pipeline");
        MU_CORE_ASSERT(spec.pipelineInfo.state->inputAssembly.has_value(), "pipeline state must have input assembly info for graphics pipeline");

        const auto &state = *spec.pipelineInfo.state;
        m_state.inputAssemblyState = state.inputAssembly->ToVk();
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

            m_shaderStages.push_back(CreateShaderStageInfo(
                static_cast<VkShaderStageFlagBits>(stage),
                shader,
                shaderInfo.entryPoint
            ));
        }

        MU_CORE_DEBUG("created graphics pipeline");
    }

    PipelineGraphics::~PipelineGraphics() {
        MU_CORE_DEBUG("destroyed graphics pipeline");
    }

    auto PipelineGraphics::Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void {
        if (m_pipeline) {
            vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
        }

        CreatePipeline(renderingCreateInfo);
    }

    auto PipelineGraphics::Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) -> void {
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_layout->Get(),
            0,
            sets.size(),
            sets.data(),
            0,
            nullptr
        );
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    auto PipelineGraphics::CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void {
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        if (m_bindingDescription) {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &m_bindingDescription.value();
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();
        } else {
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
            vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
        }

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
        pipelineCreateInfo.pStages = m_shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &m_state.inputAssemblyState;
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

        auto result = vkCreateGraphicsPipelines(m_device.GetDevice(), m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline");
    }

}
