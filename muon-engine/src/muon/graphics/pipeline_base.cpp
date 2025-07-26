#include "muon/graphics/pipeline_base.hpp"

#include "muon/core/expect.hpp"
#include "muon/fs/fs.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <utility>

namespace muon::graphics {

PipelineBase::PipelineBase(const Context &context, std::shared_ptr<PipelineLayout> layout)
    : m_context(context), m_layout(layout) {
    createCache();
}

auto PipelineBase::get() -> vk::raii::Pipeline & { return m_pipeline; }
auto PipelineBase::get() const -> const vk::raii::Pipeline & { return m_pipeline; }

auto PipelineBase::createCache() -> void {
    VkPipelineCacheCreateInfo pipelineCacheCi;
    pipelineCacheCi.flags = 0;
    pipelineCacheCi.initialDataSize = 0;
    pipelineCacheCi.pInitialData = nullptr;

    auto pipelineCacheResult = m_context.getDevice().createPipelineCache(pipelineCacheCi);
    core::expect(pipelineCacheResult, "failed to create pipeline cache");

    m_cache = std::move(*pipelineCacheResult);
}

auto PipelineBase::createShaderModule(const std::filesystem::path &path, vk::raii::ShaderModule &shaderModule) const -> void {
    auto byteCode = fs::readFileBinary(path);
    core::expect(byteCode, "code does not have value");

    vk::ShaderModuleCreateInfo shaderModuleCi;
    shaderModuleCi.codeSize = byteCode->size();
    shaderModuleCi.pCode = reinterpret_cast<const uint32_t *>(byteCode->data());

    auto shaderModuleResult = m_context.getDevice().createShaderModule(shaderModuleCi);
    core::expect(shaderModuleResult, "failed to create shader module");

    shaderModule = std::move(*shaderModuleResult);
}

auto PipelineBase::createShaderStageInfo(
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
