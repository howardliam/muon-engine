#include "muon/graphics/pipeline_layout.hpp"

#include "muon/core/assert.hpp"

namespace muon::graphics {

PipelineLayout::PipelineLayout(const Spec &spec) : m_context(*spec.context) {
    VkPipelineLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createInfo.setLayoutCount = spec.setLayouts.size();
    createInfo.pSetLayouts = spec.setLayouts.data();

    if (spec.pushConstant) {
        VkPushConstantRange range = spec.pushConstant.value();
        createInfo.pushConstantRangeCount = 1;
        createInfo.pPushConstantRanges = &range;
    } else {
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = nullptr;
    }

    auto result = vkCreatePipelineLayout(m_context.GetDevice(), &createInfo, nullptr, &m_layout);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create pipeline layout");
}

PipelineLayout::~PipelineLayout() { vkDestroyPipelineLayout(m_context.GetDevice(), m_layout, nullptr); }

VkPipelineLayout PipelineLayout::Get() const { return m_layout; }

} // namespace muon::graphics
