#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/device_context.hpp"

#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class PipelineLayout : NoCopy, NoMove {
public:
    struct Spec {
        const DeviceContext *device{nullptr};
        std::vector<VkDescriptorSetLayout> setLayouts{};
        std::optional<VkPushConstantRange> pushConstant{std::nullopt};
    };

public:
    PipelineLayout(const Spec &spec);
    ~PipelineLayout();

public:
    [[nodiscard]] VkPipelineLayout Get() const;

private:
    const DeviceContext &m_device;

    VkPipelineLayout m_layout{nullptr};
};

} // namespace muon::graphics
