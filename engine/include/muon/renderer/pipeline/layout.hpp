#pragma once

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Device;

    class PipelineLayout {
    public:
        PipelineLayout() = delete;

        PipelineLayout(
            Device &device,
            const std::vector<vk::DescriptorSetLayout> &setLayouts,
            const std::optional<vk::PushConstantRange> &pushConstant
        );
        ~PipelineLayout();

        [[nodiscard]] vk::PipelineLayout getLayout() const;

    private:
        Device &device;

        vk::PipelineLayout layout;
    };

}
