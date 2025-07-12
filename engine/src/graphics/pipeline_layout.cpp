#include "muon/graphics/pipeline_layout.hpp"

#include "muon/core/assert.hpp"

namespace muon::graphics {

PipelineLayout::PipelineLayout(const Spec &spec) : m_device(*spec.device) {
    VkPipelineLayoutCreateInfo createInfo{};
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

    auto result = vkCreatePipelineLayout(m_device.GetDevice(), &createInfo, nullptr, &m_layout);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create pipeline layout");
}

PipelineLayout::~PipelineLayout() { vkDestroyPipelineLayout(m_device.GetDevice(), m_layout, nullptr); }

VkPipelineLayout PipelineLayout::Get() const { return m_layout; }

} // namespace muon::graphics
