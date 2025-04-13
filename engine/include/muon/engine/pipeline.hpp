#pragma once

#include "muon/engine/device.hpp"
#include <filesystem>
#include <map>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

namespace muon::engine {

    namespace pipeline {
        struct ConfigInfo {
            ConfigInfo() = default;

            ConfigInfo(const ConfigInfo &) = delete;
            ConfigInfo &operator=(const ConfigInfo &) = delete;

            vk::PipelineViewportStateCreateInfo viewportState;
            vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
            vk::PipelineRasterizationStateCreateInfo rasterizationState;
            vk::PipelineMultisampleStateCreateInfo multisampleState;
            vk::PipelineColorBlendAttachmentState colorBlendAttachment;
            vk::PipelineColorBlendStateCreateInfo colorBlendState;
            vk::PipelineDepthStencilStateCreateInfo depthStencilState;
            std::vector<vk::DynamicState> dynamicStateEnables;
            vk::PipelineDynamicStateCreateInfo dynamicState;
            vk::PipelineLayout pipelineLayout = nullptr;
            vk::RenderPass renderPass = nullptr;
            uint32_t subpass = 0;
        };

        void defaultConfigInfo(pipeline::ConfigInfo &configInfo);
    }

    class Pipeline {
    public:
        class Builder {
        public:
            Builder(Device &device);

            Builder &addShader(vk::ShaderStageFlagBits stage, const std::filesystem::path &path);

            Pipeline build(const pipeline::ConfigInfo &configInfo) const;

        private:
            Device &device;

            std::map<vk::ShaderStageFlagBits, std::filesystem::path> shaderPaths;
            pipeline::ConfigInfo configInfo{};
        };

        Pipeline(Device &device, const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths, const pipeline::ConfigInfo &configInfo);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void bind(vk::CommandBuffer commandBuffer);

    private:
        Device &device;

        vk::Pipeline graphicsPipeline;
        std::vector<vk::ShaderModule> shaders;

        void createShaderModule(const std::vector<char> &byteCode, vk::ShaderModule &shaderModule);
        void createGraphicsPipeline(const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths, const pipeline::ConfigInfo &configInfo);
    };

}
