#include "muon/graphics/pipeline_compute.hpp"

#include "muon/core/assert.hpp"
#include "muon/utils/fs.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    PipelineCompute::PipelineCompute(const PipelineComputeSpecification &spec) : m_device(*spec.device), m_layout(spec.layout) {
        MU_CORE_ASSERT(spec.device, "device must not be null");
        MU_CORE_ASSERT(spec.layout, "layout must not be null");

        CreateCache();
        CreateShaderModule(spec.path);
        CreatePipeline();
    }

    PipelineCompute::~PipelineCompute() {
        vkDestroyPipeline(m_device.GetDevice(), m_pipeline, nullptr);
        vkDestroyShaderModule(m_device.GetDevice(), m_shader, nullptr);
        vkDestroyPipelineCache(m_device.GetDevice(), m_cache, nullptr);
    }

    void PipelineCompute::Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) const {
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            m_layout->Get(),
            0,
            sets.size(),
            sets.data(),
            0,
            nullptr
        );
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
    }

    void PipelineCompute::Dispatch(VkCommandBuffer cmd, const VkExtent2D &extent, const glm::uvec3 &groupCount) const {
        vkCmdDispatch(cmd, groupCount.x, groupCount.y, groupCount.z);
    }

    VkPipeline PipelineCompute::Get() const {
        return m_pipeline;
    }

    void PipelineCompute::CreateCache() {
        VkPipelineCacheCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.flags = 0;
        createInfo.initialDataSize = 0;
        createInfo.pInitialData = nullptr;

        auto result = vkCreatePipelineCache(m_device.GetDevice(), &createInfo, nullptr, &m_cache);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create compute pipeline cache");
    }

    void PipelineCompute::CreateShaderModule(const std::filesystem::path &path) {
        std::vector byteCode = fs::ReadFileBinary(path);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = byteCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(byteCode.data());

        auto result = vkCreateShaderModule(m_device.GetDevice(), &createInfo, nullptr, &m_shader);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create compute shader module");
    }

    void PipelineCompute::CreatePipeline() {
        VkPipelineShaderStageCreateInfo stageCreateInfo{};
        stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageCreateInfo.module = m_shader;
        stageCreateInfo.pName = "main";
        stageCreateInfo.flags = 0;

        VkComputePipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stage = stageCreateInfo;
        pipelineCreateInfo.layout = m_layout->Get();
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.basePipelineHandle = nullptr;

        auto result = vkCreateComputePipelines(m_device.GetDevice(), m_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create compute pipeline");
    }

}
