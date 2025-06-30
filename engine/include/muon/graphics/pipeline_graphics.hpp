#pragma once

#include "muon/graphics/pipeline_layout.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineGraphicsState {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        VkPipelineMultisampleStateCreateInfo multisampleState{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        std::vector<VkDynamicState> dynamicStateEnables{};
        VkPipelineDynamicStateCreateInfo dynamicState{};

        PipelineGraphicsState();
        ~PipelineGraphicsState() = default;
    };

    struct PipelineGraphicsShaderPaths {
        std::filesystem::path vertPath{};
        std::optional<std::filesystem::path> tescPath{std::nullopt};
        std::optional<std::filesystem::path> tesePath{std::nullopt};
        std::optional<std::filesystem::path> geomPath{std::nullopt};
        std::filesystem::path fragPath{};
    };

    struct PipelineGraphicsSpecification {
        VkDevice device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        PipelineGraphicsState state{};
        PipelineGraphicsShaderPaths paths{};
    };

    class PipelineGraphics : NoCopy, NoMove {
    public:
        PipelineGraphics(const PipelineGraphicsSpecification &spec);
        ~PipelineGraphics();

        void Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo);
        void Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets);

    private:
        void CreateCache();
        void CreateShaderModules(const PipelineGraphicsShaderPaths &paths);
        void CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo);

    private:
        VkDevice m_device{nullptr};
        std::shared_ptr<PipelineLayout> m_layout{nullptr};
        VkPipelineCache m_cache{nullptr};

        PipelineGraphicsState m_state{};

        std::vector<VkShaderModule> m_shaders{};
        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions{};
        std::optional<VkVertexInputBindingDescription> m_bindingDescription{std::nullopt};

        VkPipeline m_pipeline{nullptr};
    };

}
