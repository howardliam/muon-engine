#pragma once

#include "muon/graphics/pipeline_layout.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineMeshState {
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        VkPipelineMultisampleStateCreateInfo multisampleState{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        std::vector<VkDynamicState> dynamicStateEnables{};
        VkPipelineDynamicStateCreateInfo dynamicState{};

        PipelineMeshState();
        ~PipelineMeshState() = default;
    };

    struct PipelineMeshShaderPaths {
        std::optional<std::filesystem::path> taskPath{std::nullopt};
        std::filesystem::path meshPath{};
        std::filesystem::path fragPath{};
    };

    struct PipelineMeshSpecification {
        VkDevice device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        PipelineMeshState state{};
        PipelineMeshShaderPaths paths{};
    };

    class PipelineMesh : NoCopy, NoMove {
    public:
        PipelineMesh(const PipelineMeshSpecification &spec);
        ~PipelineMesh();

        void Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo);
        void Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets);

    private:
        void CreateCache();
        void CreateShaderModules(const PipelineMeshShaderPaths &paths);
        void CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo);

    private:
        VkDevice m_device{nullptr};
        std::shared_ptr<PipelineLayout> m_layout{nullptr};
        VkPipelineCache m_cache{nullptr};

        PipelineMeshState m_state{};

        std::vector<VkShaderModule> m_shaders{};
        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};

        VkPipeline m_pipeline{nullptr};
    };

}
