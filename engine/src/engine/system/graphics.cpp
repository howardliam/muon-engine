#include "muon/engine/system/graphics.hpp"

#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    GraphicsSystem::GraphicsSystem(
        Device &device,
        std::vector<vk::DescriptorSetLayout> setLayouts,
        std::vector<vk::PushConstantRange> pushConstants
    ) : device(device) {
        createPipelineLayout(setLayouts, pushConstants);
    }

    GraphicsSystem::~GraphicsSystem() {
        device.getDevice().destroyPipelineLayout(pipelineLayout, nullptr);
    }

    void GraphicsSystem::bake(const vk::PipelineRenderingCreateInfo &renderingInfo) {
        createPipeline(renderingInfo);
    }

    void GraphicsSystem::createPipelineLayout(
        std::vector<vk::DescriptorSetLayout> setLayouts,
        std::vector<vk::PushConstantRange> pushConstants
    ) {
        vk::PipelineLayoutCreateInfo createInfo{};
        createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        createInfo.pSetLayouts = setLayouts.data();
        createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        createInfo.pPushConstantRanges = pushConstants.data();

        auto result = device.getDevice().createPipelineLayout(&createInfo, nullptr, &pipelineLayout);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

}
