#include "muon/graphics/pipeline_graphics.hpp"

#include "muon/core/assert.hpp"
#include "muon/utils/fs.hpp"
#include <map>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    PipelineGraphicsState::PipelineGraphicsState() {
        inputAssemblyState.topology =  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyState.primitiveRestartEnable = false;

        viewportState.viewportCount= 1;
        viewportState.pViewports = nullptr;
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;

        rasterizationState.depthClampEnable = false;
        rasterizationState.rasterizerDiscardEnable = false;
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationState.lineWidth = 1.0f;
        rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationState.depthBiasEnable = false;
        rasterizationState.depthBiasConstantFactor = 0.0;
        rasterizationState.depthBiasClamp = 0.0;
        rasterizationState.depthBiasSlopeFactor = 0.0;

        multisampleState.sampleShadingEnable = false;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.minSampleShading = 1.0;
        multisampleState.pSampleMask = nullptr;
        multisampleState.alphaToCoverageEnable = false;
        multisampleState.alphaToOneEnable = false;

        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = false;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        colorBlendState.logicOpEnable = false;
        colorBlendState.logicOp = VK_LOGIC_OP_COPY;
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = &colorBlendAttachment;
        colorBlendState.blendConstants[0] = 0.0;
        colorBlendState.blendConstants[1] = 0.0;
        colorBlendState.blendConstants[2] = 0.0;
        colorBlendState.blendConstants[3] = 0.0;

        depthStencilState.depthTestEnable = true;
        depthStencilState.depthWriteEnable = true;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilState.depthBoundsTestEnable = false;
        depthStencilState.minDepthBounds = 0.0;
        depthStencilState.maxDepthBounds = 1.0;
        depthStencilState.stencilTestEnable = false;
        depthStencilState.front = VkStencilOpState{};
        depthStencilState.back = VkStencilOpState{};

        dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        dynamicState.pDynamicStates = dynamicStateEnables.data();
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    }

    PipelineGraphics::PipelineGraphics(const PipelineGraphicsSpecification &spec) : m_device(spec.device), m_layout(spec.layout), m_state(spec.state) {
        CreateCache();
        CreateShaderModules(spec.paths);
    }

    PipelineGraphics::~PipelineGraphics() {
        vkDestroyPipeline(m_device, m_pipeline, nullptr);
        for (const auto shader : m_shaders) {
            vkDestroyShaderModule(m_device, shader, nullptr);
        }
        vkDestroyPipelineCache(m_device, m_cache, nullptr);
    }

    void PipelineGraphics::CreateCache() {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;

        auto result = vkCreatePipelineCache(m_device, &createInfo, nullptr, &m_cache);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline cache");
    }

    void PipelineGraphics::CreateShaderModules(const PipelineGraphicsShaderPaths &paths) {
        std::map<VkShaderStageFlagBits, std::filesystem::path> shaderPaths;

        shaderPaths[VK_SHADER_STAGE_VERTEX_BIT] = paths.vertPath;
        if (paths.tescPath) {
            shaderPaths[VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT] = *paths.tescPath;
        }
        if (paths.tesePath) {
            shaderPaths[VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT] = *paths.tesePath;
        }
        if (paths.geomPath) {
            shaderPaths[VK_SHADER_STAGE_GEOMETRY_BIT] = *paths.geomPath;
        }
        shaderPaths[VK_SHADER_STAGE_FRAGMENT_BIT] = paths.fragPath;

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

        auto result = vkCreateGraphicsPipelines(m_device, m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create graphics pipeline");
    }

}
