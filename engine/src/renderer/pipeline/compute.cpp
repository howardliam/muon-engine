#include "muon/renderer/pipeline/compute.hpp"

#include "muon/core/assert.hpp"
#include "muon/renderer/device.hpp"
#include "muon/renderer/shader.hpp"

namespace muon {

    ComputePipeline::ComputePipeline(
        Device &device,
        const std::filesystem::path &shaderPath,
        const std::vector<vk::DescriptorSetLayout> &setLayouts,
        const std::vector<vk::PushConstantRange> &pushConstants
    ) : device(device) {
        createPipelineLayout(setLayouts, pushConstants);
        createPipeline(shaderPath);
    }

    ComputePipeline::~ComputePipeline() {
        device.device().destroyShaderModule(shader);
        device.device().destroyPipeline(pipeline);
        device.device().destroyPipelineCache(cache);
        device.device().destroyPipelineLayout(layout);
    }

    void ComputePipeline::bind(vk::CommandBuffer cmd, const std::vector<vk::DescriptorSet> &sets) {
        cmd.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            layout,
            0,
            sets.size(),
            sets.data(),
            0,
            nullptr
        );
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);
    }

    void ComputePipeline::dispatch(
        vk::CommandBuffer cmd,
        const vk::Extent2D &windowExtent,
        const std::array<uint32_t, 3> &workgroupSize
    ) {
        uint32_t groupCountX = (windowExtent.width + workgroupSize[0] - 1) / workgroupSize[0];
        uint32_t groupCountY = (windowExtent.height + workgroupSize[1] - 1) / workgroupSize[1];

        cmd.dispatch(groupCountX, groupCountY, workgroupSize[2]);
    }

    vk::Pipeline ComputePipeline::getPipeline() const {
        return pipeline;
    }

    void ComputePipeline::createShaderModule(const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

        auto result = device.device().createShaderModule(&createInfo, nullptr, &shaderModule);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create shader module");
    }

    void ComputePipeline::createPipelineLayout(
        const std::vector<vk::DescriptorSetLayout> &setLayouts,
        const std::vector<vk::PushConstantRange> &pushConstants
    ) {
        vk::PipelineLayoutCreateInfo createInfo{};
        createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        createInfo.pSetLayouts = setLayouts.data();
        createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        createInfo.pPushConstantRanges = pushConstants.data();

        auto result = device.device().createPipelineLayout(&createInfo, nullptr, &layout);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create compute pipeline layout");
    }

    void ComputePipeline::createPipeline(const std::filesystem::path &shaderPath) {
        vk::PipelineCacheCreateInfo pcCreateInfo{};
        pcCreateInfo.flags = vk::PipelineCacheCreateFlags{};
        pcCreateInfo.initialDataSize = 0;
        pcCreateInfo.pInitialData = nullptr;

        auto result = device.device().createPipelineCache(&pcCreateInfo, nullptr, &cache);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create compute pipeline cache");

        std::vector byteCode = ShaderCompiler::readFile(shaderPath);
        createShaderModule(byteCode, shader);

        vk::PipelineShaderStageCreateInfo sCreateInfo{};
        sCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
        sCreateInfo.module = shader;
        sCreateInfo.pName = "main";
        sCreateInfo.flags = vk::PipelineShaderStageCreateFlags{};

        vk::ComputePipelineCreateInfo pCreateInfo{};
        pCreateInfo.stage = sCreateInfo;
        pCreateInfo.layout = layout;
        pCreateInfo.basePipelineIndex = -1;
        pCreateInfo.basePipelineHandle = nullptr;

        result = device.device().createComputePipelines(cache, 1, &pCreateInfo, nullptr, &pipeline);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create compute pipeline");
    }

    ComputePipeline::Builder::Builder(Device &device) : device(device) {}

    ComputePipeline::Builder &ComputePipeline::Builder::setShader(const std::filesystem::path &path) {
        this->path = path;
        return *this;
    }

    ComputePipeline::Builder &ComputePipeline::Builder::setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout> &setLayouts) {
        this->setLayouts = setLayouts;
        return *this;
    }

    ComputePipeline::Builder &ComputePipeline::Builder::setPushConstants(const std::vector<vk::PushConstantRange> &pushConstants) {
        this->pushConstants = pushConstants;
        return *this;
    }

    std::unique_ptr<ComputePipeline> ComputePipeline::Builder::buildUniquePtr() {
        return std::make_unique<ComputePipeline>(device, path, setLayouts, pushConstants);
    }

}
