#include "muon/engine/pipeline.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <print>
#include <stdexcept>
#include <vector>

namespace muon::engine {

    std::vector<char> readFile(const std::filesystem::path &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error(std::format("failed to open file: {}", path.string()));
        }

        std::vector<char> buffer(file.tellg());

        file.seekg(0);
        file.read(buffer.data(), buffer.size());

        return buffer;
    }

    Pipeline::Pipeline(Device &device, const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, const pipeline::ConfigInfo &configInfo) : device(device) {
        createGraphicsPipeline(vertPath, fragPath, configInfo);
    }

    Pipeline::~Pipeline() {
        device.getDevice().destroyShaderModule(vertShader, nullptr);
        device.getDevice().destroyShaderModule(fragShader, nullptr);
        device.getDevice().destroyPipeline(graphicsPipeline, nullptr);
    }

    void Pipeline::bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    }

    void Pipeline::defaultConfigInfo(pipeline::ConfigInfo &configInfo) {
        configInfo.inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = false;

        configInfo.viewportInfo.viewportCount= 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;

        configInfo.rasterizationInfo.depthClampEnable = false;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = false;
        configInfo.rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
        configInfo.rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
        configInfo.rasterizationInfo.depthBiasEnable = false;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        configInfo.multisampleInfo.sampleShadingEnable = false;
        configInfo.multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        configInfo.multisampleInfo.minSampleShading = 1.0f;
        configInfo.multisampleInfo.pSampleMask = nullptr;
        configInfo.multisampleInfo.alphaToCoverageEnable = false;
        configInfo.multisampleInfo.alphaToOneEnable = false;

        configInfo.colorBlendAttachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;
        configInfo.colorBlendAttachment.blendEnable = false;
        configInfo.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        configInfo.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        configInfo.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        configInfo.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        configInfo.colorBlendInfo.logicOpEnable = false;
        configInfo.colorBlendInfo.logicOp = vk::LogicOp::eCopy;
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

        configInfo.depthStencilInfo.depthTestEnable = true;
        configInfo.depthStencilInfo.depthWriteEnable = true;
        configInfo.depthStencilInfo.depthCompareOp = vk::CompareOp::eLess;
        configInfo.depthStencilInfo.depthBoundsTestEnable = false;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f;
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
        configInfo.depthStencilInfo.stencilTestEnable = false;
        configInfo.depthStencilInfo.front = vk::StencilOpState{};
        configInfo.depthStencilInfo.back = vk::StencilOpState{};

        configInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        configInfo.dynamicStateInfo.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    }

    void Pipeline::createShaderModule(const std::vector<char> &byteCode, vk::ShaderModule &shaderModule) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

        auto result = device.getDevice().createShaderModule(&createInfo, nullptr, &shaderModule);
        if (result != vk::Result::eSuccess) {
            std::println("failed to create shader module");
        }
    }

    void Pipeline::createGraphicsPipeline(const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, const pipeline::ConfigInfo &configInfo) {
        auto vert = readFile(vertPath);
        auto frag = readFile(fragPath);

        createShaderModule(vert, vertShader);
        createShaderModule(frag, fragShader);

        std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

        size_t idx = 0;
        shaderStages[idx].stage = vk::ShaderStageFlagBits::eVertex;
        shaderStages[idx].module = vertShader;
        shaderStages[idx].pName = "main";
        shaderStages[idx].flags = vk::PipelineShaderStageCreateFlags{};
        shaderStages[idx].pNext = nullptr;
        shaderStages[idx].pSpecializationInfo = nullptr;

        idx += 1;
        shaderStages[idx].stage = vk::ShaderStageFlagBits::eFragment;
        shaderStages[idx].module = fragShader;
        shaderStages[idx].pName = "main";
        shaderStages[idx].flags = vk::PipelineShaderStageCreateFlags{};
        shaderStages[idx].pNext = nullptr;
        shaderStages[idx].pSpecializationInfo = nullptr;

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineCreateInfo.pViewportState = &configInfo.viewportInfo;
        pipelineCreateInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineCreateInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineCreateInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineCreateInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineCreateInfo.pDynamicState = &configInfo.dynamicStateInfo;

        pipelineCreateInfo.layout = configInfo.pipelineLayout;
        pipelineCreateInfo.renderPass = configInfo.renderPass;
        pipelineCreateInfo.subpass = configInfo.subpass;

        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = nullptr;

        auto result = device.getDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
        if (result != vk::Result::eSuccess) {
            std::println("failed to create graphics pipeline");
        }
    }

}
