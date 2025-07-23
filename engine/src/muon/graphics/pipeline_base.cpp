#include "muon/graphics/pipeline_base.hpp"

#include "muon/core/assert.hpp"
#include "muon/fs/fs.hpp"

namespace muon::graphics {

PipelineBase::PipelineBase(const Context &context, std::shared_ptr<PipelineLayout> layout)
    : m_context(context), m_layout(layout) {
    CreateCache();
}

PipelineBase::~PipelineBase() {
    vkDestroyPipeline(m_context.GetDevice(), m_pipeline, nullptr);
    for (const auto &shader : m_shaders) {
        vkDestroyShaderModule(m_context.GetDevice(), shader, nullptr);
    }
    vkDestroyPipelineCache(m_context.GetDevice(), m_cache, nullptr);
}

auto PipelineBase::CreateCache() -> void {
    VkPipelineCacheCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.initialDataSize = 0;
    createInfo.pInitialData = nullptr;

    auto result = vkCreatePipelineCache(m_context.GetDevice(), &createInfo, nullptr, &m_cache);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create pipeline cache");
}

auto PipelineBase::CreateShaderModule(const std::filesystem::path &path, VkShaderModule &shaderModule) const -> void {
    auto byteCode = fs::ReadFileBinary(path);
    MU_CORE_ASSERT(byteCode.has_value(), "code does not have value");

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = byteCode->size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode->data());

    auto result = vkCreateShaderModule(m_context.GetDevice(), &createInfo, nullptr, &shaderModule);
    MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create shader module");
}

auto PipelineBase::CreateShaderStageInfo(
    const VkShaderStageFlagBits stage, const VkShaderModule &shaderModule, const std::string_view entryPoint
) const -> VkPipelineShaderStageCreateInfo {
    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = stage;
    info.module = shaderModule;
    info.pName = entryPoint.data();
    info.flags = 0;
    return info;
}

} // namespace muon::graphics
