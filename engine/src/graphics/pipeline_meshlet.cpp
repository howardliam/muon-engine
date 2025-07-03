#include "muon/graphics/pipeline_meshlet.hpp"

#include "muon/core/assert.hpp"
#include "muon/schematic/pipeline/common.hpp"
#include "muon/utils/fs.hpp"
#include <map>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    PipelineMeshlet::PipelineMeshlet(const PipelineMeshletSpecification &spec) : m_device(spec.device), m_layout(spec.layout) {
        MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Meshlet, "must be meshlet pipeline config");
        MU_CORE_ASSERT(spec.pipelineInfo.IsValid(), "must be a valid meshlet pipeline config");
        MU_CORE_ASSERT(spec.pipelineInfo.state.has_value(), "pipeline state must exist for meshlet pipeline");

        const auto &state = *spec.pipelineInfo.state;

        m_state.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        m_state.viewportState.viewportCount = state.viewport.viewportCount;
        m_state.viewportState.pViewports = nullptr;
        m_state.viewportState.scissorCount = state.viewport.scissorCount;
        m_state.viewportState.pScissors = nullptr;

        m_state.rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_state.rasterizationState.polygonMode = static_cast<VkPolygonMode>(state.rasterization.polygonMode);
        if (state.rasterization.polygonMode == schematic::PolygonMode::Line) {
            m_state.rasterizationState.lineWidth = *state.rasterization.lineWidth;
        }
        m_state.rasterizationState.cullMode = static_cast<VkCullModeFlagBits>(state.rasterization.cullMode);
        m_state.rasterizationState.frontFace = static_cast<VkFrontFace>(state.rasterization.frontFace);
        m_state.rasterizationState.rasterizerDiscardEnable = state.rasterization.rasterizerDiscardEnable;
        m_state.rasterizationState.depthClampEnable = state.rasterization.depthClampEnable;
        m_state.rasterizationState.depthBiasEnable = state.rasterization.depthBiasEnable;
        if (state.rasterization.depthBiasEnable) {
            m_state.rasterizationState.depthBiasConstantFactor = *state.rasterization.depthBiasConstantFactor;
            m_state.rasterizationState.depthBiasClamp = *state.rasterization.depthBiasClamp;
            m_state.rasterizationState.depthBiasSlopeFactor = *state.rasterization.depthBiasSlopeFactor;
        }

        m_state.multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_state.multisampleState.rasterizationSamples = static_cast<VkSampleCountFlagBits>(state.multisample.rasterizationSamples);
        m_state.multisampleState.sampleShadingEnable = state.multisample.sampleShadingEnable;
        if (state.multisample.sampleShadingEnable) {
            m_state.multisampleState.minSampleShading = *state.multisample.minSampleShading;
        }
        m_state.multisampleState.alphaToCoverageEnable = state.multisample.alphaToCoverageEnable;
        m_state.multisampleState.alphaToOneEnable = state.multisample.alphaToOneEnable;
        m_state.multisampleState.pSampleMask = nullptr;

        m_state.colorBlendAttachments.reserve(state.colorBlend.attachments.size());
        for (const auto &attachment : state.colorBlend.attachments) {
            VkPipelineColorBlendAttachmentState att{};
            att.blendEnable = attachment.blendEnable;
            if (attachment.blendEnable) {
                att.srcColorBlendFactor = static_cast<VkBlendFactor>(*attachment.srcColorBlendFactor);
                att.dstColorBlendFactor = static_cast<VkBlendFactor>(*attachment.dstColorBlendFactor);
                att.colorBlendOp = static_cast<VkBlendOp>(*attachment.colorBlendOp);
                att.srcAlphaBlendFactor = static_cast<VkBlendFactor>(*attachment.srcAlphaBlendFactor);
                att.dstAlphaBlendFactor = static_cast<VkBlendFactor>(*attachment.dstAlphaBlendFactor);
                att.alphaBlendOp = static_cast<VkBlendOp>(*attachment.colorBlendOp);
                att.colorWriteMask = attachment.colorWriteMask->to_ulong();
            }

            m_state.colorBlendAttachments.push_back(att);
        }

        m_state.colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_state.colorBlendState.logicOpEnable = state.colorBlend.logicOpEnable;
        if (state.colorBlend.logicOpEnable) {
            m_state.colorBlendState.logicOp = static_cast<VkLogicOp>(*state.colorBlend.logicOp);
        }
        m_state.colorBlendState.attachmentCount = m_state.colorBlendAttachments.size();
        m_state.colorBlendState.pAttachments = m_state.colorBlendAttachments.data();
        m_state.colorBlendState.blendConstants[0] = state.colorBlend.blendConstants[0];
        m_state.colorBlendState.blendConstants[1] = state.colorBlend.blendConstants[1];
        m_state.colorBlendState.blendConstants[2] = state.colorBlend.blendConstants[2];
        m_state.colorBlendState.blendConstants[3] = state.colorBlend.blendConstants[3];

        m_state.depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        m_state.depthStencilState.depthTestEnable = state.depthStencil.depthTestEnable;
        if (state.depthStencil.depthTestEnable) {
            m_state.depthStencilState.depthWriteEnable = *state.depthStencil.depthWriteEnable;
            m_state.depthStencilState.depthCompareOp = static_cast<VkCompareOp>(*state.depthStencil.depthCompareOp);
            m_state.depthStencilState.depthBoundsTestEnable = *state.depthStencil.depthBoundsTestEnable;
            m_state.depthStencilState.minDepthBounds = *state.depthStencil.minDepthBounds;
            m_state.depthStencilState.maxDepthBounds = *state.depthStencil.maxDepthBounds;
        }

        m_state.depthStencilState.stencilTestEnable = state.depthStencil.stencilTestEnable;
        if (state.depthStencil.stencilTestEnable) {
            m_state.depthStencilState.front = VkStencilOpState{};
            m_state.depthStencilState.back = VkStencilOpState{};
        }

        m_state.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        m_state.dynamicStateEnables.reserve(state.dynamic.states.size());
        for (const auto &dynamic : state.dynamic.states) {
            m_state.dynamicStateEnables.push_back(static_cast<VkDynamicState>(dynamic));
        }
        m_state.dynamicState.pDynamicStates = m_state.dynamicStateEnables.data();
        m_state.dynamicState.dynamicStateCount = static_cast<uint32_t>(m_state.dynamicStateEnables.size());

        CreateCache();
        CreateShaderModules(spec.pipelineInfo.shaders);
    }

    PipelineMeshlet::~PipelineMeshlet() {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        for (const auto shader : m_shaders) {
            vkDestroyShaderModule(m_device, shader, nullptr);
        }
        vkDestroyPipelineCache(m_device, m_cache, nullptr);
    }

    void PipelineMeshlet::Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) {
        if (m_pipeline) {
            vkDestroyPipeline(m_device, m_pipeline, nullptr);
        }

        CreatePipeline(renderingCreateInfo);
    }

    void PipelineMeshlet::Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) {
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

    void PipelineMeshlet::CreateCache() {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;

        auto result = vkCreatePipelineCache(m_device, &createInfo, nullptr, &m_cache);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline cache");
    }

    void PipelineMeshlet::CreateShaderModules(const std::unordered_map<schematic::ShaderStage, schematic::ShaderInfo> &shaders) {
        std::map<VkShaderStageFlagBits, std::filesystem::path> shaderPaths;

        if (shaders.contains(schematic::ShaderStage::Task)) {
            shaderPaths[VK_SHADER_STAGE_TASK_BIT_EXT] = *shaders.find(schematic::ShaderStage::Task)->second.path;
        }
        shaderPaths[VK_SHADER_STAGE_MESH_BIT_EXT] = *shaders.find(schematic::ShaderStage::Mesh)->second.path;
        shaderPaths[VK_SHADER_STAGE_FRAGMENT_BIT] = *shaders.find(schematic::ShaderStage::Fragment)->second.path;

        m_shaders.resize(shaderPaths.size());
        m_shaderStages.resize(shaderPaths.size());

        uint32_t index = 0;
        for (const auto &[stage, path] : shaderPaths) {
            std::vector byteCode = fs::ReadFileBinary(path);

            VkShaderModuleCreateInfo shaderCreateInfo{};
            shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderCreateInfo.codeSize = byteCode.size();
            shaderCreateInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

            auto result = vkCreateShaderModule(m_device, &shaderCreateInfo, nullptr, &m_shaders[index]);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create compute shader module");

            VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage = stage;
            shaderStageCreateInfo.module = m_shaders[index];
            shaderStageCreateInfo.pName = "main";
            shaderStageCreateInfo.flags = 0;
            shaderStageCreateInfo.pNext = nullptr;
            shaderStageCreateInfo.pSpecializationInfo = nullptr;

            m_shaderStages[index] = shaderStageCreateInfo;

            index += 1;
        }
    }

    void PipelineMeshlet::CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo) {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
        pipelineCreateInfo.pStages = m_shaderStages.data();
        pipelineCreateInfo.pVertexInputState = nullptr;
        pipelineCreateInfo.pInputAssemblyState = nullptr;
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

        auto result = vkCreateGraphicsPipelines(m_device, m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline");
    }

}
