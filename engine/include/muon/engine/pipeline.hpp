#pragma once

#include "muon/engine/device.hpp"
#include <filesystem>
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
    }

    class Pipeline {
    public:
        Pipeline(Device &device, const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, const pipeline::ConfigInfo &configInfo);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        Pipeline &operator=(const Pipeline &) = delete;

        void bind(vk::CommandBuffer commandBuffer);

        static void defaultConfigInfo(pipeline::ConfigInfo &configInfo);

    private:
        Device &device;
        vk::Pipeline graphicsPipeline;
        vk::ShaderModule vertShader;
        vk::ShaderModule fragShader;

        void createShaderModule(const std::vector<char> &byteCode, vk::ShaderModule &shaderModule);
        void createGraphicsPipeline(const std::filesystem::path &vertPath, const std::filesystem::path &fragPath, const pipeline::ConfigInfo &configInfo);
    };

    class Pipeline2 {
    public:
        class Builder {
        public:

        private:
        };


    private:

    };

}
