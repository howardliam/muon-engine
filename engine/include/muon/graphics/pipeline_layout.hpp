#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class PipelineLayout : NoCopy, NoMove {
public:
    struct Spec {
        const Context *context{nullptr};
        std::vector<VkDescriptorSetLayout> setLayouts{};
        std::optional<VkPushConstantRange> pushConstant{std::nullopt};
    };

public:
    PipelineLayout(const Spec &spec);
    ~PipelineLayout();

public:
    VkPipelineLayout Get() const;

private:
    const Context &m_context;

    VkPipelineLayout m_layout{nullptr};
};

} // namespace muon::graphics
