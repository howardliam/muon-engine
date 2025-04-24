#pragma once

#include "muon/engine/device.hpp"
#include <filesystem>

namespace muon::engine {

    class ComputePipeline {
    public:
        ComputePipeline(
            Device &device,
            const std::filesystem::path &shaderPath,
            const vk::PipelineLayout pipelineLayout
        );
        ~ComputePipeline();

    private:
        Device &device;

        vk::Pipeline pipeline;
        vk::ShaderModule shader;

        void createShaderModule(const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule);

        void createPipeline(
            const std::filesystem::path &shaderPath,
            const vk::PipelineLayout pipelineLayout
        );
    };

}
