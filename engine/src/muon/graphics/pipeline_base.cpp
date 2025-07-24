#include "muon/graphics/pipeline_base.hpp"

#include "muon/core/expect.hpp"
#include "muon/fs/fs.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <utility>

namespace muon::graphics {

auto PipelineBase::Get() -> vk::raii::Pipeline & { return m_pipeline; }
auto PipelineBase::Get() const -> const vk::raii::Pipeline & { return m_pipeline; }

PipelineBase::PipelineBase(const Context &context, std::shared_ptr<PipelineLayout> layout)
    : m_context(context), m_layout(layout) {
    CreateCache();
}

auto PipelineBase::CreateCache() -> void {
    VkPipelineCacheCreateInfo pipelineCacheCi;
    pipelineCacheCi.flags = 0;
    pipelineCacheCi.initialDataSize = 0;
    pipelineCacheCi.pInitialData = nullptr;

    auto pipelineCacheResult = m_context.GetDevice().createPipelineCache(pipelineCacheCi);
    core::expect(pipelineCacheResult, "failed to create pipeline cache");

    m_cache = std::move(*pipelineCacheResult);
}

auto PipelineBase::CreateShaderModule(const std::filesystem::path &path, vk::raii::ShaderModule &shaderModule) const -> void {
    auto byteCode = fs::ReadFileBinary(path);
    core::expect(byteCode, "code does not have value");

    vk::ShaderModuleCreateInfo shaderModuleCi;
    shaderModuleCi.codeSize = byteCode->size();
    shaderModuleCi.pCode = reinterpret_cast<const uint32_t *>(byteCode->data());

    auto shaderModuleResult = m_context.GetDevice().createShaderModule(shaderModuleCi);
    core::expect(shaderModuleResult, "failed to create shader module");

    shaderModule = std::move(*shaderModuleResult);
}

auto PipelineBase::CreateShaderStageInfo(
    const vk::ShaderStageFlagBits stage, const vk::raii::ShaderModule &shaderModule, const std::string_view entryPoint
) const -> vk::PipelineShaderStageCreateInfo {
    vk::PipelineShaderStageCreateInfo shaderStageCi;
    shaderStageCi.stage = stage;
    shaderStageCi.module = shaderModule;
    shaderStageCi.pName = entryPoint.data();
    shaderStageCi.flags = vk::PipelineShaderStageCreateFlags{};

    return shaderStageCi;
}

} // namespace muon::graphics
