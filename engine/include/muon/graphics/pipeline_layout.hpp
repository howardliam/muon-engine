#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineLayoutSpecification {
        const DeviceContext *device{nullptr};
        std::vector<VkDescriptorSetLayout> setLayouts{};
        std::optional<VkPushConstantRange> pushConstant{std::nullopt};
    };

    class PipelineLayout : NoCopy, NoMove {
    public:
        PipelineLayout(const PipelineLayoutSpecification &spec);
        ~PipelineLayout();

    public:
        [[nodiscard]] VkPipelineLayout Get() const;

    private:
        const DeviceContext &m_device;

        VkPipelineLayout m_layout{nullptr};
    };

}
