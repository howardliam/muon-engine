#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <filesystem>
#include <memory>
#include <string_view>

namespace muon::graphics {

class PipelineBase : NoCopy, NoMove {
public:
    PipelineBase(const Context &context, std::shared_ptr<PipelineLayout> layout);

public:
    auto get() -> vk::raii::Pipeline &;
    auto get() const -> const vk::raii::Pipeline &;

protected:
    auto createCache() -> void;
    auto createShaderModule(const std::filesystem::path &path, vk::raii::ShaderModule &shaderModule) const -> void;
    auto createShaderStageInfo(
        const vk::ShaderStageFlagBits stage, const vk::raii::ShaderModule &shaderModule, const std::string_view entryPoint
    ) const -> vk::PipelineShaderStageCreateInfo;

protected:
    const Context &m_context;

    std::shared_ptr<PipelineLayout> m_layout{nullptr};
    vk::raii::PipelineCache m_cache{nullptr};
    std::vector<vk::raii::ShaderModule> m_shaders{};
    vk::raii::Pipeline m_pipeline{nullptr};
};

} // namespace muon::graphics
