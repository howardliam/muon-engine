#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <array>
#include <filesystem>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace muon {

    class Device;

    class ComputePipeline : NoCopy, NoMove {
    public:
        class Builder;

        ComputePipeline(
            Device &device,
            const std::filesystem::path &shaderPath,
            const std::vector<vk::DescriptorSetLayout> &setLayouts,
            const std::vector<vk::PushConstantRange> &pushConstants
        );
        ~ComputePipeline();

        void bind(vk::CommandBuffer cmd, const std::vector<vk::DescriptorSet> &sets);

        void dispatch(
            vk::CommandBuffer cmd,
            const vk::Extent2D &windowExtent,
            const std::array<uint32_t, 3> &workgroupSize
        );

        [[nodiscard]] vk::Pipeline getPipeline() const;

    private:
        Device &device;

        vk::PipelineLayout layout;
        vk::Pipeline pipeline;
        vk::PipelineCache cache;
        vk::ShaderModule shader;

        void createShaderModule(const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule);

        void createPipelineLayout(
            const std::vector<vk::DescriptorSetLayout> &setLayouts,
            const std::vector<vk::PushConstantRange> &pushConstants
        );

        void createPipeline(const std::filesystem::path &shaderPath);
    };

    class ComputePipeline::Builder {
    public:
        Builder(Device &device);

        Builder &setShader(const std::filesystem::path &path);

        Builder &setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout> &setLayouts);

        Builder &setPushConstants(const std::vector<vk::PushConstantRange> &pushConstants);

        std::unique_ptr<ComputePipeline> buildUniquePtr();

    private:
        Device &device;

        std::filesystem::path path;
        std::vector<vk::DescriptorSetLayout> setLayouts;
        std::vector<vk::PushConstantRange> pushConstants;
    };

}
