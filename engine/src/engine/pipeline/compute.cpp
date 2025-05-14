#include "muon/engine/pipeline/compute.hpp"

#include "muon/engine/device.hpp"
#include "muon/engine/shader.hpp"

namespace muon::engine {

    ComputePipeline::ComputePipeline(
        Device &device,
        const std::filesystem::path &shaderPath,
        const vk::PipelineLayout pipelineLayout
    ) : device(device) {
        createPipeline(shaderPath, pipelineLayout);
    }

    ComputePipeline::~ComputePipeline() {
        device.getDevice().destroyShaderModule(shader);
        device.getDevice().destroyPipeline(pipeline);
        device.getDevice().destroyPipelineCache(cache);
    }

    vk::Pipeline ComputePipeline::getPipeline() const {
        return pipeline;
    }

    void ComputePipeline::createShaderModule(const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule) {
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

        auto result = device.getDevice().createShaderModule(&createInfo, nullptr, &shaderModule);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void ComputePipeline::createPipeline(
        const std::filesystem::path &shaderPath,
        const vk::PipelineLayout pipelineLayout
    ) {
        vk::PipelineCacheCreateInfo pcCreateInfo{};
        pcCreateInfo.flags = vk::PipelineCacheCreateFlags{};
        pcCreateInfo.initialDataSize = 0;
        pcCreateInfo.pInitialData = nullptr;

        auto result = device.getDevice().createPipelineCache(&pcCreateInfo, nullptr, &cache);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create compute pipeline cache");
        }

        std::vector byteCode = ShaderCompiler::readFile(shaderPath);
        createShaderModule(byteCode, shader);

        vk::PipelineShaderStageCreateInfo sCreateInfo{};
        sCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
        sCreateInfo.module = shader;
        sCreateInfo.pName = "main";
        sCreateInfo.flags = vk::PipelineShaderStageCreateFlags{};

        vk::ComputePipelineCreateInfo pCreateInfo{};
        pCreateInfo.stage = sCreateInfo;
        pCreateInfo.layout = pipelineLayout;
        pCreateInfo.basePipelineIndex = -1;
        pCreateInfo.basePipelineHandle = nullptr;

        result = device.getDevice().createComputePipelines(cache, 1, &pCreateInfo, nullptr, &pipeline);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

}
