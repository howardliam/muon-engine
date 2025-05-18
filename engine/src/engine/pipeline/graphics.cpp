#include "muon/engine/pipeline/graphics.hpp"

#include "muon/engine/pipeline/layout.hpp"
#include "muon/engine/shader.hpp"
#include "muon/engine/device.hpp"
#include "muon/log/logger.hpp"
#include <algorithm>
#include <memory>
#include <spirv_reflect.h>

namespace {
    using VertexInputState = std::tuple<std::vector<vk::VertexInputAttributeDescription>, std::optional<vk::VertexInputBindingDescription>>;
}

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

        log::globalLogger->trace("input var count: {}", inputVarCount);

        auto sorter = [](SpvReflectInterfaceVariable *a, SpvReflectInterfaceVariable *b) -> bool {
            return a->location < b->location;
        };
        std::sort(inputVars.begin(), inputVars.end(), sorter);

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(inputVarCount);

        size_t offset{0};
        for (const auto &inputVar : inputVars) {
            size_t index = inputVar->location;
            log::globalLogger->trace("setting location: {}", inputVar->location);
            attributeDescriptions[index].location = inputVar->location;
            log::globalLogger->trace("setting binding");
            attributeDescriptions[index].binding = 0;
            log::globalLogger->trace("setting format");
            attributeDescriptions[index].format = static_cast<vk::Format>(inputVar->format);
            log::globalLogger->trace("setting offset: {}", offset);
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
        std::unique_ptr<ConfigInfo> &&configInfo,
        const std::vector<vk::DescriptorSetLayout> &setLayouts,
        const std::optional<vk::PushConstantRange> &pushConstant,
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths
    ) : device(device), configInfo(std::move(configInfo)) {
        layout = std::make_shared<PipelineLayout>(device, setLayouts, pushConstant);
        createPipelineCache();
        createShaderModules(shaderPaths);
    }

    GraphicsPipeline::GraphicsPipeline(
        Device &device,
        std::unique_ptr<ConfigInfo> &&configInfo,
        std::shared_ptr<PipelineLayout> layout,
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths
    ) : device(device), configInfo(std::move(configInfo)), layout(layout) {
        createPipelineCache();
        createShaderModules(shaderPaths);
    }

    GraphicsPipeline::~GraphicsPipeline() {
        device.getDevice().destroyPipeline(pipeline);

        for (const auto shader : shaders) {
            device.getDevice().destroyShaderModule(shader);
        }

        device.getDevice().destroyPipelineCache(cache);
    }

    void GraphicsPipeline::bake(const vk::PipelineRenderingCreateInfo &renderingInfo) {
        if (pipeline != nullptr) {
            device.getDevice().destroyPipeline(pipeline);
        }

        createPipeline(renderingInfo);
    }

    void GraphicsPipeline::bind(vk::CommandBuffer cmd, const std::vector<vk::DescriptorSet> &sets) {
        cmd.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            layout->getLayout(),
            0,
            sets.size(),
            sets.data(),
            0,
            nullptr
        );

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }

    vk::PipelineLayout GraphicsPipeline::getLayout() const {
        return layout->getLayout();
    }

    vk::Pipeline GraphicsPipeline::getPipeline() const {
        return pipeline;
    }

    void GraphicsPipeline::createPipelineLayout(
        const std::vector<vk::DescriptorSetLayout> &setLayouts,
        const std::optional<vk::PushConstantRange> &pushConstant
    ) {
        // vk::PipelineLayoutCreateInfo plCreateInfo{};
        // plCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        // plCreateInfo.pSetLayouts = setLayouts.data();
        // plCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        // plCreateInfo.pPushConstantRanges = pushConstants.data();

        // auto result = device.getDevice().createPipelineLayout(&plCreateInfo, nullptr, &layout);
        // if (result != vk::Result::eSuccess) {
        //     throw std::runtime_error("failed to create graphics pipeline layout");
        // }
    }

    void GraphicsPipeline::createPipelineCache() {
        vk::PipelineCacheCreateInfo pcCreateInfo{};
        pcCreateInfo.flags = vk::PipelineCacheCreateFlags{};
        pcCreateInfo.initialDataSize = 0;
        pcCreateInfo.pInitialData = nullptr;

        auto result = device.getDevice().createPipelineCache(&pcCreateInfo, nullptr, &cache);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline cache");
        }
    }

    void GraphicsPipeline::createShaderModules(
        const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths
    ) {
        auto createShaderModule = [&](const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule) {
            vk::ShaderModuleCreateInfo smCreateInfo;
            smCreateInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
            smCreateInfo.codeSize = byteCode.size();
            smCreateInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

            auto result = device.getDevice().createShaderModule(&smCreateInfo, nullptr, &shaderModule);
            if (result != vk::Result::eSuccess) {
                throw std::runtime_error("failed to create shader module");
            }
        };

        shaders.resize(shaderPaths.size());
        shaderStages.resize(shaderPaths.size());

        size_t index = 0;
        for (auto &[stage, path] : shaderPaths) {
            std::vector byteCode = ShaderCompiler::readFile(path);
            vk::ShaderModule shaderModule;
            createShaderModule(byteCode, shaderModule);
            shaders[index] = shaderModule;

            if (stage == vk::ShaderStageFlagBits::eVertex) {
                auto [attr, bind] = vertexInputStateFromSpirv(byteCode);

                attributeDescriptions = attr;
                bindingDescription = bind;
            }

            vk::PipelineShaderStageCreateInfo pssCreateInfo{};
            pssCreateInfo.stage = stage;
            pssCreateInfo.module = shaderModule;
            pssCreateInfo.pName = "main";
            pssCreateInfo.flags = vk::PipelineShaderStageCreateFlags{};
            pssCreateInfo.pNext = nullptr;
            pssCreateInfo.pSpecializationInfo = nullptr;

            shaderStages[index] = pssCreateInfo;

            index += 1;
        }
    }

    void GraphicsPipeline::createPipeline(const vk::PipelineRenderingCreateInfo &renderingInfo) {
        vk::PipelineVertexInputStateCreateInfo pvCreateInfo{};

        if (bindingDescription.has_value()) {
            pvCreateInfo.vertexBindingDescriptionCount = 1;
            pvCreateInfo.pVertexBindingDescriptions = &bindingDescription.value();
            pvCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            pvCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        } else {
            pvCreateInfo.vertexBindingDescriptionCount = 0;
            pvCreateInfo.pVertexBindingDescriptions = nullptr;
            pvCreateInfo.vertexAttributeDescriptionCount = 0;
            pvCreateInfo.pVertexAttributeDescriptions = nullptr;
        }

        vk::GraphicsPipelineCreateInfo gpCreateInfo{};
        gpCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        gpCreateInfo.pStages = shaderStages.data();
        gpCreateInfo.pVertexInputState = &pvCreateInfo;
        gpCreateInfo.pInputAssemblyState = &configInfo->inputAssemblyState;
        gpCreateInfo.pViewportState = &configInfo->viewportState;
        gpCreateInfo.pRasterizationState = &configInfo->rasterizationState;
        gpCreateInfo.pMultisampleState = &configInfo->multisampleState;
        gpCreateInfo.pColorBlendState = &configInfo->colorBlendState;
        gpCreateInfo.pDepthStencilState = &configInfo->depthStencilState;
        gpCreateInfo.pDynamicState = &configInfo->dynamicState;

        gpCreateInfo.layout = layout->getLayout();
        gpCreateInfo.pNext = &renderingInfo;
        gpCreateInfo.subpass = 0;

        gpCreateInfo.basePipelineIndex = -1;
        gpCreateInfo.basePipelineHandle = nullptr;

        auto result = device.getDevice().createGraphicsPipelines(cache, 1, &gpCreateInfo, nullptr, &pipeline);
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
        configInfo.dynamicState.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicState.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    }

    GraphicsPipeline::Builder::Builder(Device &device) : device(device) {
        configInfo = std::make_unique<ConfigInfo>();
        defaultConfigInfo(*configInfo);
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::addShader(vk::ShaderStageFlagBits stage, const std::filesystem::path &path) {
        assert(stage < vk::ShaderStageFlagBits::eCompute);
        shaderPaths[stage] = path;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setDescriptorSetLayouts(
        const std::vector<vk::DescriptorSetLayout> &setLayouts
    ) {
        this->setLayouts = setLayouts;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setPushConstant(
        const vk::PushConstantRange &pushConstant
    ) {
        this->pushConstant = pushConstant;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setPipelineLayout(
        const std::shared_ptr<PipelineLayout> &pipelineLayout
    ) {
        this->pipelineLayout = pipelineLayout;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setInputAssemblyState(
        const vk::PipelineInputAssemblyStateCreateInfo &state
    ) {
        configInfo->inputAssemblyState = state;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setViewportState(
        const vk::PipelineViewportStateCreateInfo &state
    ) {
        configInfo->viewportState = state;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setRasterizationState(
        const vk::PipelineRasterizationStateCreateInfo &state
    ) {
        configInfo->rasterizationState = state;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setMultisampleState(
        const vk::PipelineMultisampleStateCreateInfo &state
    ) {
        configInfo->multisampleState = state;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setColorBlendAttachmentState(
        const vk::PipelineColorBlendAttachmentState &state
    ) {
        configInfo->colorBlendAttachment = state;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setColorBlendState(
        const vk::PipelineColorBlendStateCreateInfo &state
    ) {
        configInfo->colorBlendState = state;
        return *this;
    }

    GraphicsPipeline::Builder &GraphicsPipeline::Builder::setDepthStencilState(
        const vk::PipelineDepthStencilStateCreateInfo &state
    ) {
        configInfo->depthStencilState = state;
        return *this;
    }

    std::unique_ptr<GraphicsPipeline> GraphicsPipeline::Builder::buildUniquePtr() {
        if (pipelineLayout) {
            return std::make_unique<GraphicsPipeline>(device, std::move(configInfo), pipelineLayout, shaderPaths);
        }

        return std::make_unique<GraphicsPipeline>(device, std::move(configInfo), setLayouts, pushConstant, shaderPaths);
    }

}
