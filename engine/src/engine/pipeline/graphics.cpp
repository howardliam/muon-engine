#include "muon/engine/pipeline/graphics.hpp"

#include "muon/engine/shader.hpp"
#include "muon/engine/device.hpp"
#include "muon/log/logger.hpp"
#include <algorithm>
#include <spirv_reflect.h>

namespace muon::engine {

    uint32_t offsetFromSpirvFormat(SpvReflectFormat format) {
        switch (format) {
        case SPV_REFLECT_FORMAT_UNDEFINED:
            return 0;

        case SPV_REFLECT_FORMAT_R16_UINT:
        case SPV_REFLECT_FORMAT_R16_SINT:
        case SPV_REFLECT_FORMAT_R16_SFLOAT:
            return 2;

        case SPV_REFLECT_FORMAT_R16G16_UINT:
        case SPV_REFLECT_FORMAT_R16G16_SINT:
        case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
            return 2 * 2;

        case SPV_REFLECT_FORMAT_R16G16B16_UINT:
        case SPV_REFLECT_FORMAT_R16G16B16_SINT:
        case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:
            return 3 * 2;

        case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
        case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
        case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
            return 4 * 2;

        case SPV_REFLECT_FORMAT_R32_UINT:
        case SPV_REFLECT_FORMAT_R32_SINT:
        case SPV_REFLECT_FORMAT_R32_SFLOAT:
            return 4;

        case SPV_REFLECT_FORMAT_R32G32_UINT:
        case SPV_REFLECT_FORMAT_R32G32_SINT:
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
            return 2 * 4;

        case SPV_REFLECT_FORMAT_R32G32B32_UINT:
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
            return 3 * 4;

        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
            return 4 * 4;

        case SPV_REFLECT_FORMAT_R64_UINT:
        case SPV_REFLECT_FORMAT_R64_SINT:
        case SPV_REFLECT_FORMAT_R64_SFLOAT:
            return 8;

        case SPV_REFLECT_FORMAT_R64G64_UINT:
        case SPV_REFLECT_FORMAT_R64G64_SINT:
        case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
            return 2 * 8;

        case SPV_REFLECT_FORMAT_R64G64B64_UINT:
        case SPV_REFLECT_FORMAT_R64G64B64_SINT:
        case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
            return 3 * 8;

        case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:
        case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:
        case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:
            return 4 * 8;

        default:
            return 0;
        }
    }

    using VertexInputState = std::tuple<std::vector<vk::VertexInputAttributeDescription>, std::optional<vk::VertexInputBindingDescription>>;
    VertexInputState vertexInputStateFromSpirv(const std::vector<uint8_t> &byteCode) {
        spv_reflect::ShaderModule module(byteCode);
        if (module.GetResult() != SPV_REFLECT_RESULT_SUCCESS) {
            log::globalLogger->warn("failed to load SPIR-V for reflection");
            return {};
        }

        uint32_t inputVarCount{0};
        module.EnumerateInputVariables(&inputVarCount, nullptr);
        std::vector<SpvReflectInterfaceVariable *> inputVars(inputVarCount);
        module.EnumerateInputVariables(&inputVarCount, inputVars.data());

        log::globalLogger->info("input var count: {}", inputVarCount);

        auto sorter = [](SpvReflectInterfaceVariable *a, SpvReflectInterfaceVariable *b) -> bool {
            return a->location < b->location;
        };
        std::sort(inputVars.begin(), inputVars.end(), sorter);

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(inputVarCount);

        size_t offset{0};
        for (const auto &inputVar : inputVars) {
            size_t index = inputVar->location;
            log::globalLogger->info("setting location: {}", inputVar->location);
            attributeDescriptions[index].location = inputVar->location;
            log::globalLogger->info("setting binding");
            attributeDescriptions[index].binding = 0;
            log::globalLogger->info("setting format");
            attributeDescriptions[index].format = static_cast<vk::Format>(inputVar->format);
            log::globalLogger->info("setting offset: {}", offset);
            attributeDescriptions[index].offset = offset;

            offset += offsetFromSpirvFormat(inputVar->format);
        }

        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;
        bindingDescription.stride = offset;

        return {std::move(attributeDescriptions), bindingDescription};
    }

    GraphicsPipeline::GraphicsPipeline(
        Device &device,
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths,
        const ConfigInfo &configInfo
    ) : device(device) {
        createPipeline(shaderPaths, configInfo);
    }

    GraphicsPipeline::~GraphicsPipeline() {
        for (const auto shader : shaders) {
            device.getDevice().destroyShaderModule(shader, nullptr);
        }

        device.getDevice().destroyPipeline(pipeline, nullptr);
    }

    void GraphicsPipeline::bind(vk::CommandBuffer commandBuffer) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

    void GraphicsPipeline::createShaderModule(const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

        auto result = device.getDevice().createShaderModule(&createInfo, nullptr, &shaderModule);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void GraphicsPipeline::createPipeline(
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths,
        const ConfigInfo &configInfo
    ) {
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        std::optional<vk::VertexInputBindingDescription> bindingDescription;

        shaders.resize(shaderPaths.size());
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(shaderPaths.size());

        size_t index = 0;
        for (auto &[stage, path] : shaderPaths) {
            std::vector byteCode = readShaderFile(path);
            vk::ShaderModule shaderModule;
            createShaderModule(byteCode, shaderModule);
            shaders[index] = shaderModule;

            if (stage == vk::ShaderStageFlagBits::eVertex) {
                auto [attr, bind] = vertexInputStateFromSpirv(byteCode);

                attributeDescriptions = attr;
                bindingDescription = bind;
            }

            vk::PipelineShaderStageCreateInfo stageCreateInfo{};
            stageCreateInfo.stage = stage;
            stageCreateInfo.module = shaderModule;
            stageCreateInfo.pName = "main";
            stageCreateInfo.flags = vk::PipelineShaderStageCreateFlags{};
            stageCreateInfo.pNext = nullptr;
            stageCreateInfo.pSpecializationInfo = nullptr;

            shaderStages[index] = stageCreateInfo;

            index += 1;
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputState{};
        if (bindingDescription.has_value()) {
            vertexInputState.vertexBindingDescriptionCount = 1;
            vertexInputState.pVertexBindingDescriptions = &bindingDescription.value();
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();
        } else {
            vertexInputState.vertexBindingDescriptionCount = 0;
            vertexInputState.pVertexBindingDescriptions = nullptr;
            vertexInputState.vertexAttributeDescriptionCount = 0;
            vertexInputState.pVertexAttributeDescriptions = nullptr;
        }

        vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputState;
        pipelineCreateInfo.pInputAssemblyState = &configInfo.inputAssemblyState;
        pipelineCreateInfo.pViewportState = &configInfo.viewportState;
        pipelineCreateInfo.pRasterizationState = &configInfo.rasterizationState;
        pipelineCreateInfo.pMultisampleState = &configInfo.multisampleState;
        pipelineCreateInfo.pColorBlendState = &configInfo.colorBlendState;
        pipelineCreateInfo.pDepthStencilState = &configInfo.depthStencilState;
        pipelineCreateInfo.pDynamicState = &configInfo.dynamicState;

        pipelineCreateInfo.layout = configInfo.pipelineLayout;
        pipelineCreateInfo.pNext = &configInfo.renderingInfo;
        pipelineCreateInfo.subpass = configInfo.subpass;

        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = nullptr;

        auto result = device.getDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

    void GraphicsPipeline::defaultConfigInfo(ConfigInfo &configInfo) {
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
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
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
        configInfo.colorBlendState.blendConstants = vk::ArrayWrapper1D<float, 4>({ 0.0f, 0.0f, 0.0f, 0.0f });

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

    GraphicsPipeline::Builder::Builder(Device &device) : device(device) {}

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::addShader(vk::ShaderStageFlagBits stage, const std::filesystem::path &path) {
        assert(stage < vk::ShaderStageFlagBits::eCompute);
        shaderPaths[stage] = path;
        return *this;
    }

    GraphicsPipeline GraphicsPipeline::Builder::build(const ConfigInfo &configInfo) const {
        return GraphicsPipeline(device, shaderPaths, configInfo);
    }

    std::unique_ptr<GraphicsPipeline> GraphicsPipeline::Builder::buildUniquePtr(const ConfigInfo &configInfo) const {
        return std::make_unique<GraphicsPipeline>(device, shaderPaths, configInfo);
    }

}
