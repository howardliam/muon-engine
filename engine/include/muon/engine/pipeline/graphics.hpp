#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <map>
#include <filesystem>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class Device;

    class GraphicsPipeline : NoCopy, NoMove {
    public:
        struct ConfigInfo;
        class Builder;

        GraphicsPipeline(
            Device &device,
            const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths,
            const ConfigInfo &configInfo
        );
        ~GraphicsPipeline();

        /**
         * @brief   binds the pipeline into the command buffer to be used by models for rendering.
         *
         * @param   commandBuffer   the command buffer to record this command into.
         */
        void bind(vk::CommandBuffer commandBuffer);

        static void defaultConfigInfo(ConfigInfo &configInfo);

    private:
        Device &device;

        vk::Pipeline pipeline;
        std::vector<vk::ShaderModule> shaders;

        /**
         * @brief   creates a shader module from the byte code.
         *
         * @param   byteCode        SPIR-V byte code to create the shader module with.
         * @param   shaderModule    shader module handle.
         */
        void createShaderModule(const std::vector<uint8_t> &byteCode, vk::ShaderModule &shaderModule);

        /**
         * @brief   creates a graphics pipeline from the shaders and config info provided.
         *
         * @param   shaderPaths ordered map of shaders.
         * @param   configInfo  config info to create the pipeline with.
         */
        void createPipeline(
            const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths,
            const ConfigInfo &configInfo
        );
    };

    struct GraphicsPipeline::ConfigInfo {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        std::vector<vk::DynamicState> dynamicStateEnables;
        vk::PipelineDynamicStateCreateInfo dynamicState;
        vk::PipelineLayout pipelineLayout = nullptr;
        vk::PipelineRenderingCreateInfo renderingInfo;
        uint32_t subpass = 0;
    };

    class GraphicsPipeline::Builder {
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
         * @return  new Pipeline object.
         */
        GraphicsPipeline build(const ConfigInfo &configInfo) const;

        /**
         * @brief   builds the pipeline from the provided info.
         *
         * @param   configInfo  config information for the pipeline to be created with.
         *
         * @return  unique pointer to new Pipeline object.
         */
        std::unique_ptr<GraphicsPipeline> buildUniquePtr(const ConfigInfo &configInfo) const;

    private:
        Device &device;

        std::map<vk::ShaderStageFlagBits, std::filesystem::path> shaderPaths;
        ConfigInfo configInfo{};
    };

}
