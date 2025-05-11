#include "muon/engine/system/compute.hpp"

#include "muon/engine/device.hpp"
#include <stdexcept>
#include <glm/vec3.hpp>

namespace muon::engine {

    ComputeSystem::ComputeSystem(Device &device, std::vector<vk::DescriptorSetLayout> setLayouts) : device(device) {
        createPipelineLayout(setLayouts);
    }

    ComputeSystem::~ComputeSystem() {
        device.getDevice().destroyPipelineLayout(pipelineLayout, nullptr);
    }

    glm::uvec3 ComputeSystem::calculateDispatchSize(vk::Extent2D windowExtent, const glm::uvec3 &workgroupSize) {
        glm::uvec3 dispatch{0};

        dispatch.x = (windowExtent.width + workgroupSize.x - 1) / workgroupSize.x;
        dispatch.y = (windowExtent.height + workgroupSize.y - 1) / workgroupSize.y;
        dispatch.z = workgroupSize.z;

        return dispatch;
    }

    void ComputeSystem::createPipelineLayout(std::vector<vk::DescriptorSetLayout> setLayouts) {
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
