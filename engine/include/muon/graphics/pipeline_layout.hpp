#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineLayoutSpecification {
        VkDevice device{nullptr};
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
        VkPipelineLayout m_layout{nullptr};
    };

}
