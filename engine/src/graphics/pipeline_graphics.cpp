#include "muon/graphics/pipeline_graphics.hpp"

#include "muon/core/assert.hpp"
#include "muon/utils/fs.hpp"
#include <map>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    PipelineGraphics::PipelineGraphics(const PipelineGraphicsSpecification &spec) : m_device(*spec.device), m_layout(spec.layout) {
        MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Graphics, "must be graphics pipeline config");
        MU_CORE_ASSERT(spec.pipelineInfo.IsValid(), "must be a valid graphics pipeline config");
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

        CreateCache();
        CreateShaderModules(spec.pipelineInfo.shaders);
    }

    PipelineGraphics::~PipelineGraphics() {
        vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
        for (const auto shader : m_shaders) {
            vkDestroyShaderModule(m_device.GetDevice(), shader, nullptr);
        }
        vkDestroyPipelineCache(m_device.GetDevice(), m_cache, nullptr);
    }

    void PipelineGraphics::Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) {
        if (m_pipeline) {
            vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
        }

        CreatePipeline(renderingCreateInfo);
    }

    void PipelineGraphics::Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) {
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

    void PipelineGraphics::CreateCache() {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;

        auto result = vkCreatePipelineCache(m_device.GetDevice(), &createInfo, nullptr, &m_cache);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline cache");
    }

    void PipelineGraphics::CreateShaderModules(const std::unordered_map<schematic::ShaderStage, schematic::ShaderInfo> &shaders) {
        std::map<VkShaderStageFlagBits, std::filesystem::path> shaderPaths;

        shaderPaths[VK_SHADER_STAGE_VERTEX_BIT] = *shaders.find(schematic::ShaderStage::Vertex)->second.path;
        if (shaders.contains(schematic::ShaderStage::TessellationControl)) {
            shaderPaths[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = *shaders.find(schematic::ShaderStage::TessellationControl)->second.path;
        }
        if (shaders.contains(schematic::ShaderStage::TessellationEvaluation)) {
            shaderPaths[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = *shaders.find(schematic::ShaderStage::TessellationEvaluation)->second.path;
        }
        if (shaders.contains(schematic::ShaderStage::Geometry)) {
            shaderPaths[VK_SHADER_STAGE_GEOMETRY_BIT] = *shaders.find(schematic::ShaderStage::Geometry)->second.path;
        }
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

            auto result = vkCreateShaderModule(m_device.GetDevice(), &shaderCreateInfo, nullptr, &m_shaders[index]);
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

    void PipelineGraphics::CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo) {
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
