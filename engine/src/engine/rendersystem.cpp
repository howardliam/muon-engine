#include "muon/engine/rendersystem.hpp"

#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    RenderSystem::RenderSystem(Device &device, std::vector<vk::DescriptorSetLayout> setLayouts) : device(device) {
        createPipelineLayout(setLayouts);
    }

    RenderSystem::~RenderSystem() {
        device.getDevice().destroyPipelineLayout(pipelineLayout, nullptr);
    }

    void RenderSystem::bake(vk::RenderPass renderPass) {
        createPipeline(renderPass);
    }

    void RenderSystem::createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts) {
        vk::PipelineLayoutCreateInfo createInfo{};
        createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        createInfo.pSetLayouts = setLayouts.data();
        createInfo.pushConstantRangeCount = 0;
        createInfo.pPushConstantRanges = nullptr;

        auto result = device.getDevice().createPipelineLayout(&createInfo, nullptr, &pipelineLayout);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

}
