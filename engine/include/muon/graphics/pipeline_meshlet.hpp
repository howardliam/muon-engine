#pragma once

#include "muon/graphics/context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/pipeline_info.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class PipelineMeshlet : PipelineBase {
public:
    struct Spec {
        const Context *context{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        schematic::PipelineInfo pipelineInfo{};
    };

public:
    PipelineMeshlet(const Spec &spec);
    ~PipelineMeshlet();

    auto Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void;
    auto Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) -> void;

private:
    auto CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void;

private:
    struct PipelineMeshletState {
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        VkPipelineMultisampleStateCreateInfo multisampleState{};
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        std::vector<VkDynamicState> dynamicStateEnables{};
        VkPipelineDynamicStateCreateInfo dynamicState{};
    };
    PipelineMeshletState m_state{};

    std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
};

} // namespace muon::graphics
