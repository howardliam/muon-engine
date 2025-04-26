#include "muon/engine/pipeline/graphics.hpp"

#include "muon/engine/pipeline.hpp"

namespace muon::engine {

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
        shaderPaths[stage] = path;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::addVertexAttribute(vk::Format format) {
        auto offsetFromFormat = [](vk::Format &format) {
            switch (format) {
            case vk::Format::eR32G32B32A32Sfloat:
            case vk::Format::eR32G32B32A32Sint:
            case vk::Format::eR32G32B32A32Uint:
                return 16;

            case vk::Format::eR32G32B32Sfloat:
            case vk::Format::eR32G32B32Uint:
            case vk::Format::eR32G32B32Sint:
                return 12;

            case vk::Format::eR32G32Sfloat:
            case vk::Format::eR32G32Uint:
            case vk::Format::eR32G32Sint:
                return 8;

            case vk::Format::eR32Sfloat:
            case vk::Format::eR32Uint:
            case vk::Format::eR32Sint:
                return 4;

            default:
                throw std::runtime_error("unsupported vertex input format");
            }
        };

        vertexLayout.attributeDescriptions.push_back({
            location,
            0,
            format,
            offset,
        });

        location += 1;
        offset += offsetFromFormat(format);

        updateBindingDescription();

        return *this;
    }

    void GraphicsPipeline::Builder::updateBindingDescription() {
        if (!vertexLayout.bindingDescription.has_value()) {
            vertexLayout.bindingDescription = vk::VertexInputBindingDescription{};

            vertexLayout.bindingDescription->binding = 0;
            vertexLayout.bindingDescription->inputRate = vk::VertexInputRate::eVertex;
        }

        vertexLayout.bindingDescription->stride = offset;
    }

    GraphicsPipeline GraphicsPipeline::Builder::build(const ConfigInfo &configInfo) const {

        return GraphicsPipeline(device, shaderPaths, vertexLayout, configInfo);
    }

    std::unique_ptr<GraphicsPipeline> GraphicsPipeline::Builder::buildUniquePointer(const ConfigInfo &configInfo) const {
        return std::make_unique<GraphicsPipeline>(device, shaderPaths, vertexLayout, configInfo);
    }

    GraphicsPipeline::GraphicsPipeline(
        Device &device,
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths,
        const VertexLayout &vertexLayout,
        const ConfigInfo &configInfo
    ) : device(device) {
        createPipeline(shaderPaths, vertexLayout, configInfo);
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
        const VertexLayout &vertexLayout,
        const ConfigInfo &configInfo
    ) {
        shaders.resize(shaderPaths.size());
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages(shaderPaths.size());

        bool hasVertShader = false;
        bool hasFragShader = false;

        size_t idx = 0;
        for (auto [stage, path] : shaderPaths) {
            if (stage == vk::ShaderStageFlagBits::eVertex) {
                hasVertShader = true;
            } else if (stage == vk::ShaderStageFlagBits::eFragment) {
                hasFragShader = true;
            }

            std::vector byteCode = readShaderFile(path);
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

        if ((!hasVertShader && !hasFragShader)) {
            throw std::runtime_error("cannot create a pipeline without vertex and fragment shaders");
        }

        vk::PipelineVertexInputStateCreateInfo vertexInputState{};
        if (vertexLayout.bindingDescription.has_value()) {
            vertexInputState.vertexBindingDescriptionCount = 1;
            vertexInputState.pVertexBindingDescriptions = &vertexLayout.bindingDescription.value();
            vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexLayout.attributeDescriptions.size());
            vertexInputState.pVertexAttributeDescriptions = vertexLayout.attributeDescriptions.data();
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
        pipelineCreateInfo.renderPass = configInfo.renderPass;
        pipelineCreateInfo.subpass = configInfo.subpass;

        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = nullptr;

        auto result = device.getDevice().createGraphicsPipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

}
