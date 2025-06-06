#pragma once

#include "muon/renderer/pipeline/layout.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <map>
#include <filesystem>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Device;

    class GraphicsPipeline : NoCopy, NoMove {
    public:
        class Builder;
        struct ConfigInfo;

        GraphicsPipeline(
            Device &device,
            std::unique_ptr<ConfigInfo> &&configInfo,
            const std::vector<vk::DescriptorSetLayout> &setLayouts,
            const std::optional<vk::PushConstantRange> &pushConstant,
            const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths
        );
        GraphicsPipeline(
            Device &device,
            std::unique_ptr<ConfigInfo> &&configInfo,
            std::shared_ptr<PipelineLayout> layout,
            const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths
        );

        ~GraphicsPipeline();

        void bake(const vk::PipelineRenderingCreateInfo &renderingInfo);

        void bind(vk::CommandBuffer cmd, const std::vector<vk::DescriptorSet> &sets);

        [[nodiscard]] vk::PipelineLayout getLayout() const;

        [[nodiscard]] vk::Pipeline getPipeline() const;

        static void defaultConfigInfo(ConfigInfo &configInfo);

    private:
        Device &device;

        std::unique_ptr<ConfigInfo> configInfo;

        // vk::PipelineLayout layout;
        std::shared_ptr<PipelineLayout> layout;
        vk::PipelineCache cache;

        std::vector<vk::ShaderModule> shaders;
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
        std::optional<vk::VertexInputBindingDescription> bindingDescription;

        vk::Pipeline pipeline;

        void createPipelineLayout(
            const std::vector<vk::DescriptorSetLayout> &setLayouts,
            const std::optional<vk::PushConstantRange> &pushConstant
        );

        void createPipelineCache();

        void createShaderModules(const std::map<vk::ShaderStageFlagBits, std::filesystem::path> &shaderPaths);

        void createPipeline(const vk::PipelineRenderingCreateInfo &renderingInfo);
    };

    class GraphicsPipeline::Builder {
    public:
        Builder(Device &device);

        Builder &addShader(vk::ShaderStageFlagBits stage, const std::filesystem::path &path);

        Builder &setDescriptorSetLayouts(const std::vector<vk::DescriptorSetLayout> &setLayouts);

        Builder &setPushConstant(const vk::PushConstantRange &pushConstant);

        Builder &setPipelineLayout(const std::shared_ptr<PipelineLayout> &pipelineLayout);

        Builder &setInputAssemblyState(const vk::PipelineInputAssemblyStateCreateInfo &state);

        Builder &setViewportState(const vk::PipelineViewportStateCreateInfo &state);

        Builder &setRasterizationState(const vk::PipelineRasterizationStateCreateInfo &state);

        Builder &setMultisampleState(const vk::PipelineMultisampleStateCreateInfo &state);

        Builder &setColorBlendAttachmentState(const vk::PipelineColorBlendAttachmentState &state);

        Builder &setColorBlendState(const vk::PipelineColorBlendStateCreateInfo &state);

        Builder &setDepthStencilState(const vk::PipelineDepthStencilStateCreateInfo &state);

        std::unique_ptr<GraphicsPipeline> buildUniquePtr();

    private:
        Device &device;

        std::map<vk::ShaderStageFlagBits, std::filesystem::path> shaderPaths;
        std::vector<vk::DescriptorSetLayout> setLayouts;
        std::optional<vk::PushConstantRange> pushConstant;
        std::shared_ptr<PipelineLayout> pipelineLayout;
        std::unique_ptr<ConfigInfo> configInfo;
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
        uint32_t subpass = 0;
    };

}
