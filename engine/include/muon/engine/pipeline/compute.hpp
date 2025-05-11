#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <filesystem>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class Device;

    class ComputePipeline : NoCopy, NoMove {
    public:
        ComputePipeline(
            Device &device,
            const std::filesystem::path &shaderPath,
            const vk::PipelineLayout pipelineLayout
        );
        ~ComputePipeline();

        [[nodiscard]] vk::Pipeline getPipeline() const;

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
