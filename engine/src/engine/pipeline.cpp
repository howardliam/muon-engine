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

    void pipeline::defaultConfigInfo(pipeline::ConfigInfo &configInfo) {
        configInfo.inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
        configInfo.inputAssemblyState.primitiveRestartEnable = false;

        configInfo.viewportState.viewportCount= 1;
        configInfo.viewportState.pViewports = nullptr;
        configInfo.viewportState.scissorCount = 1;
        configInfo.viewportState.pScissors = nullptr;

        configInfo.rasterizationState.depthClampEnable = false;
        configInfo.rasterizationState.rasterizerDiscardEnable = false;
        configInfo.rasterizationState.polygonMode = vk::PolygonMode::eFill;
        configInfo.rasterizationState.lineWidth = 1.0f;
        configInfo.rasterizationState.cullMode = vk::CullModeFlagBits::eNone;
        configInfo.rasterizationState.frontFace = vk::FrontFace::eClockwise;
        configInfo.rasterizationState.depthBiasEnable = false;
        configInfo.rasterizationState.depthBiasConstantFactor = 0.0f;
        configInfo.rasterizationState.depthBiasClamp = 0.0f;
        configInfo.rasterizationState.depthBiasSlopeFactor = 0.0f;

        configInfo.multisampleState.sampleShadingEnable = false;
        configInfo.multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;
        configInfo.multisampleState.minSampleShading = 1.0f;
        configInfo.multisampleState.pSampleMask = nullptr;
        configInfo.multisampleState.alphaToCoverageEnable = false;
        configInfo.multisampleState.alphaToOneEnable = false;

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

        configInfo.colorBlendState.logicOpEnable = false;
        configInfo.colorBlendState.logicOp = vk::LogicOp::eCopy;
        configInfo.colorBlendState.attachmentCount = 1;
        configInfo.colorBlendState.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendState.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

        configInfo.depthStencilState.depthTestEnable = true;
        configInfo.depthStencilState.depthWriteEnable = true;
        configInfo.depthStencilState.depthCompareOp = vk::CompareOp::eLess;
        configInfo.depthStencilState.depthBoundsTestEnable = false;
        configInfo.depthStencilState.minDepthBounds = 0.0f;
        configInfo.depthStencilState.maxDepthBounds = 1.0f;
        configInfo.depthStencilState.stencilTestEnable = false;
        configInfo.depthStencilState.front = vk::StencilOpState{};
        configInfo.depthStencilState.back = vk::StencilOpState{};

        configInfo.dynamicStateEnables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        configInfo.dynamicState.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        configInfo.dynamicState.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicState.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    }

    Pipeline::Builder::Builder(Device &device) : device(device) {}

    Pipeline::Builder &Pipeline::Builder::addShader(vk::ShaderStageFlagBits stage, const std::filesystem::path &path) {
        shaderPaths[stage] = path;
        return *this;
    }

    Pipeline Pipeline::Builder::build(const pipeline::ConfigInfo &configInfo) const {
        return Pipeline(device, shaderPaths, configInfo);
    }

    Pipeline::Pipeline(
        Device &device,
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths,
        const pipeline::ConfigInfo &configInfo
    ) : device(device) {
        createGraphicsPipeline(shaderPaths, configInfo);
    }

    Pipeline::~Pipeline() {
        for (const auto shader : shaders) {
            device.getDevice().destroyShaderModule(shader, nullptr);
        }

        device.getDevice().destroyPipeline(graphicsPipeline, nullptr);
    }

    void Pipeline::bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
    }

    void Pipeline::createShaderModule(const std::vector<char> &byteCode, vk::ShaderModule &shaderModule) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

        auto result = device.getDevice().createShaderModule(&createInfo, nullptr, &shaderModule);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void Pipeline::createGraphicsPipeline(const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths, const pipeline::ConfigInfo &configInfo) {
        shaders.resize(shaderPaths.size());
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(shaderPaths.size());

        size_t idx = 0;
        for (auto [stage, path] : shaderPaths) {
            std::vector byteCode = readFile(path);
            vk::ShaderModule shaderModule;
            createShaderModule(byteCode, shaderModule);
            shaders[idx] = shaderModule;

            vk::PipelineShaderStageCreateInfo stageCreateInfo{};
            stageCreateInfo.stage = stage;
            stageCreateInfo.module = shaderModule;
            stageCreateInfo.pName = "main";
            stageCreateInfo.flags = vk::PipelineShaderStageCreateFlags{};
            stageCreateInfo.pNext = nullptr;
            stageCreateInfo.pSpecializationInfo = nullptr;

            shaderStages[idx] = stageCreateInfo;

            idx += 1;
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
        pipelineCreateInfo.pInputAssemblyState = &configInfo.inputAssemblyState;
        pipelineCreateInfo.pViewportState = &configInfo.viewportState;
        pipelineCreateInfo.pRasterizationState = &configInfo.rasterizationState;
        pipelineCreateInfo.pMultisampleState = &configInfo.multisampleState;
        pipelineCreateInfo.pColorBlendState = &configInfo.colorBlendState;
        pipelineCreateInfo.pDepthStencilState = &configInfo.depthStencilState;
        pipelineCreateInfo.pDynamicState = &configInfo.dynamicState;

        pipelineCreateInfo.layout = configInfo.pipelineLayout;
        pipelineCreateInfo.renderPass = configInfo.renderPass;
        pipelineCreateInfo.subpass = configInfo.subpass;

        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = nullptr;

        auto result = device.getDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }
}
