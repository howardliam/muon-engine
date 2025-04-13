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

        /**
         * @brief   loads default config info into struct.
         *
         * @param   configInfo  reference to struct.
         */
        void defaultConfigInfo(pipeline::ConfigInfo &configInfo);
    }

    /**
     * @brief   wrapper around pipeline.
     */
    class Pipeline {
    public:
        class Builder {
        public:
            Builder(Device &device);

            /**
             * @brief   adds a shader at the stage.
             *
             * @param   stage   the stage to add the shader for.
             * @param   path    path to the shader SPIR-V file.
             *
             * @return  reference to Builder.
             */
            Builder &addShader(vk::ShaderStageFlagBits stage, const std::filesystem::path &path);

            /**
             * @brief   builds the pipeline from the provided info.
             *
             * @param   configInfo  config information for the pipeline to be created with.
             *
             * @return  reference to Builder.
             */
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

        /**
         * @brief   binds the pipeline into the command buffer to be used by models for rendering.
         *
         * @param   commandBuffer   the command buffer to record this command into.
         */
        void bind(vk::CommandBuffer commandBuffer);

    private:
        Device &device;

        vk::Pipeline graphicsPipeline;
        std::vector<vk::ShaderModule> shaders;

        /**
         * @brief   creates a shader module from the byte code.
         *
         * @param   byteCode        SPIR-V byte code to create the shader module with.
         * @param   shaderModule    shader module handle.
         */
        void createShaderModule(const std::vector<char> &byteCode, vk::ShaderModule &shaderModule);

        /**
         * @brief   creates a graphics pipeline from the shaders and config info provided.
         *
         * @param   shaderPaths ordered map of shaders.
         * @param   configInfo  config info to create the pipeline with.
         */
        void createGraphicsPipeline(const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths, const pipeline::ConfigInfo &configInfo);
    };

}
