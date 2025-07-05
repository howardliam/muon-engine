#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/pipeline_info.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineMeshletSpecification {
        const DeviceContext *device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        schematic::PipelineInfo pipelineInfo{};
    };

    class PipelineMeshlet : PipelineBase {
    public:
        PipelineMeshlet(const PipelineMeshletSpecification &spec);
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

}
