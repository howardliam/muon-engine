#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/pipeline_layout.hpp"

#include <filesystem>
#include <memory>
#include <string_view>

namespace muon::graphics {

class PipelineBase : NoCopy, NoMove {
protected:
    PipelineBase(const Context &context, std::shared_ptr<PipelineLayout> layout);
    ~PipelineBase();

    auto CreateCache() -> void;
    auto CreateShaderModule(const std::filesystem::path &path, VkShaderModule &shaderModule) const -> void;
    [[nodiscard]] auto CreateShaderStageInfo(
        const VkShaderStageFlagBits stage, const VkShaderModule &shaderModule, const std::string_view entryPoint
    ) const -> VkPipelineShaderStageCreateInfo;

protected:
    const Context &m_context;
    std::shared_ptr<PipelineLayout> m_layout{nullptr};
    VkPipelineCache m_cache{nullptr};
    std::vector<VkShaderModule> m_shaders{};
    VkPipeline m_pipeline{nullptr};
};

} // namespace muon::graphics
