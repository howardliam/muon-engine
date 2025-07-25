#pragma once

#include "muon/graphics/context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <memory>

namespace muon::graphics {

class PipelineGraphics : PipelineBase {
public:
    struct Spec {
        const Context *context{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
    };

public:
    PipelineGraphics(const Spec &spec);
    ~PipelineGraphics();

    auto bake(const vk::PipelineRenderingCreateInfo &renderingCreateInfo) -> void;
    auto bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) -> void;

private:
    auto createPipeline(const vk::PipelineRenderingCreateInfo &renderingCreateInfo) -> void;

private:
    struct PipelineGraphicsState {
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
        vk::PipelineViewportStateCreateInfo viewportState{};
        vk::PipelineRasterizationStateCreateInfo rasterizationState{};
        vk::PipelineMultisampleStateCreateInfo multisampleState{};
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{};
        vk::PipelineColorBlendStateCreateInfo colorBlendState{};
        vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
        std::vector<vk::DynamicState> dynamicStateEnables{};
        vk::PipelineDynamicStateCreateInfo dynamicState{};
    };
    PipelineGraphicsState m_state{};

    std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages{};
    std::vector<vk::VertexInputAttributeDescription> m_attributeDescriptions{};
    std::optional<vk::VertexInputBindingDescription> m_bindingDescription{std::nullopt};
};

} // namespace muon::graphics
