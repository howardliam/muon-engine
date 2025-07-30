#pragma once

#include "muon/graphics/context.hpp"
#include "muon/graphics/pipeline_base.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <memory>

namespace muon::graphics {

class PipelineMeshlet : PipelineBase {
public:
    struct State {
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        std::vector<vk::DynamicState> dynamicStateEnables;
        vk::PipelineDynamicStateCreateInfo dynamicState;
    };

    struct Spec {
        const Context &context;

        std::shared_ptr<PipelineLayout> layout{nullptr};
        State state;

        Spec(const Context &context) : context{context} {}
    };

public:
    PipelineMeshlet(const Spec &spec);
    ~PipelineMeshlet();

    auto bake(const vk::PipelineRenderingCreateInfo &renderingCi) -> void;
    auto bind(vk::raii::CommandBuffer &commandBuffer, const std::vector<vk::DescriptorSet> &sets) -> void;

private:
    auto createPipeline(const vk::PipelineRenderingCreateInfo &renderingCi) -> void;

private:
    State m_state{};

    std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages{};
};

} // namespace muon::graphics
