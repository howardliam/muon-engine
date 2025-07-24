#include "muon/graphics/pipeline_layout.hpp"

#include "muon/core/expect.hpp"

#include <utility>

namespace muon::graphics {

PipelineLayout::PipelineLayout(const Spec &spec) : m_context(*spec.context) {
    vk::PipelineLayoutCreateInfo pipelineLayoutCi;
    pipelineLayoutCi.setLayoutCount = spec.setLayouts.size();
    pipelineLayoutCi.pSetLayouts = spec.setLayouts.data();

    if (spec.pushConstant) {
        vk::PushConstantRange range = spec.pushConstant.value();
        pipelineLayoutCi.pushConstantRangeCount = 1;
        pipelineLayoutCi.pPushConstantRanges = &range;
    } else {
        pipelineLayoutCi.pushConstantRangeCount = 0;
        pipelineLayoutCi.pPushConstantRanges = nullptr;
    }

    auto pipelineLayoutResult = m_context.GetDevice().createPipelineLayout(pipelineLayoutCi);
    core::expect(pipelineLayoutResult, "failed to create pipeline layout");

    m_layout = std::move(*pipelineLayoutResult);
}

auto PipelineLayout::Get() -> vk::raii::PipelineLayout & { return m_layout; }
auto PipelineLayout::Get() const -> const vk::raii::PipelineLayout & { return m_layout; }

} // namespace muon::graphics
