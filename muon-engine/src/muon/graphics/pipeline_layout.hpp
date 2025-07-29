#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <optional>
#include <vector>

namespace muon::graphics {

class PipelineLayout : NoCopy, NoMove {
public:
    struct Spec {
        const Context &context;

        std::vector<vk::DescriptorSetLayout> setLayouts{};
        std::optional<vk::PushConstantRange> pushConstant{std::nullopt};

        Spec(const Context &context) : context{context} {}
    };

public:
    PipelineLayout(const Spec &spec);

public:
    auto get() -> vk::raii::PipelineLayout &;
    auto get() const -> const vk::raii::PipelineLayout &;

private:
    const Context &m_context;

    vk::raii::PipelineLayout m_layout{nullptr};
};

} // namespace muon::graphics
