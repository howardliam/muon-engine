#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/pipeline_info.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineGraphicsSpecification {
        const DeviceContext *device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        schematic::PipelineInfo pipelineInfo{};
    };

    class PipelineGraphics : PipelineBase {
    public:
        PipelineGraphics(const PipelineGraphicsSpecification &spec);
        ~PipelineGraphics();

        auto Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void;
        auto Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) -> void;

    private:
        auto CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo) -> void;

    private:
        struct PipelineGraphicsState {
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
            VkPipelineViewportStateCreateInfo viewportState{};
            VkPipelineRasterizationStateCreateInfo rasterizationState{};
            VkPipelineMultisampleStateCreateInfo multisampleState{};
            std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
            VkPipelineColorBlendStateCreateInfo colorBlendState{};
            VkPipelineDepthStencilStateCreateInfo depthStencilState{};
            std::vector<VkDynamicState> dynamicStateEnables{};
            VkPipelineDynamicStateCreateInfo dynamicState{};
        };
        PipelineGraphicsState m_state{};

        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions{};
        std::optional<VkVertexInputBindingDescription> m_bindingDescription{std::nullopt};
    };

}
