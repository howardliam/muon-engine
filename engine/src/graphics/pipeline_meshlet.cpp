#include "muon/graphics/pipeline_meshlet.hpp"

#include "muon/core/assert.hpp"
#include "muon/schematic/pipeline/common.hpp"
#include "muon/utils/fs.hpp"
#include <map>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    PipelineMeshlet::PipelineMeshlet(const PipelineMeshletSpecification &spec) : m_device(*spec.device), m_layout(spec.layout) {
        MU_CORE_ASSERT(spec.pipelineInfo.type == schematic::PipelineType::Meshlet, "must be meshlet pipeline config");
        MU_CORE_ASSERT(spec.pipelineInfo.IsValid(), "must be a valid meshlet pipeline config");
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

        CreateCache();
        CreateShaderModules(spec.pipelineInfo.shaders);
    }

    PipelineMeshlet::~PipelineMeshlet() {
        vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
        for (const auto shader : m_shaders) {
            vkDestroyShaderModule(m_device.GetDevice(), shader, nullptr);
        }
        vkDestroyPipelineCache(m_device.GetDevice(), m_cache, nullptr);
    }

    void PipelineMeshlet::Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) {
        if (m_pipeline) {
            vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
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

        auto result = vkCreatePipelineCache(m_device.GetDevice(), &createInfo, nullptr, &m_cache);
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

        auto result = vkCreateGraphicsPipelines(m_device.GetDevice(), m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline");
    }

}
