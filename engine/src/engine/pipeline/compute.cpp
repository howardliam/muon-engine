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
        device.getDevice().destroyShaderModule(shader, nullptr);
        device.getDevice().destroyPipeline(pipeline, nullptr);
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
        std::vector byteCode = ShaderCompiler::readFile(shaderPath);
        createShaderModule(byteCode, shader);

        vk::PipelineShaderStageCreateInfo stageCreateInfo{};
        stageCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;
        stageCreateInfo.module = shader;
        stageCreateInfo.pName = "main";
        stageCreateInfo.flags = vk::PipelineShaderStageCreateFlags{};

        vk::ComputePipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.stage = stageCreateInfo;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = nullptr;

        auto result = device.getDevice().createComputePipelines(nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create graphics pipeline");
        }
    }

}
