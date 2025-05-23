#include "muon/engine/renderer/pipeline/layout.hpp"

#include "muon/engine/renderer/device.hpp"

namespace mu {

    PipelineLayout::PipelineLayout(
        Device &device,
        const std::vector<vk::DescriptorSetLayout> &setLayouts,
        const std::optional<vk::PushConstantRange> &pushConstant
    ) : device(device) {
        vk::PipelineLayoutCreateInfo createInfo{};
        createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        createInfo.pSetLayouts = setLayouts.data();

        if (pushConstant.has_value()) {
            vk::PushConstantRange range = pushConstant.value();
            createInfo.pushConstantRangeCount = 1;
            createInfo.pPushConstantRanges = &range;
        } else {
            createInfo.pushConstantRangeCount = 0;
            createInfo.pPushConstantRanges = nullptr;
        }

        auto result = device.getDevice().createPipelineLayout(&createInfo, nullptr, &layout);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create compute pipeline layout");
        }
    }

    PipelineLayout::~PipelineLayout() {
        device.getDevice().destroyPipelineLayout(layout);
    }

    [[nodiscard]] vk::PipelineLayout PipelineLayout::getLayout() const {
        return layout;
    }

}
