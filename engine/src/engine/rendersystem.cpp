#include "muon/engine/rendersystem.hpp"

#include "muon/engine/pipeline.hpp"
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    RenderSystem::RenderSystem(Device &device, std::vector<vk::DescriptorSetLayout> setLayouts, vk::RenderPass renderPass) : device(device) {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);
    }

    RenderSystem::~RenderSystem() {
        device.getDevice().destroyPipelineLayout(pipelineLayout, nullptr);
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

    void RenderSystem::createPipeline(vk::RenderPass renderPass) {
        pipeline::ConfigInfo config{};
        pipeline::defaultConfigInfo(config);

        config.renderPass = renderPass;
        config.pipelineLayout = pipelineLayout;

        // pipeline = std::make_unique<Pipeline>()
    }


}
