#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/shader_info.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <string_view>

namespace muon::graphics {

    class PipelineBase : NoCopy, NoMove {
    protected:
        PipelineBase(const DeviceContext &device, std::shared_ptr<PipelineLayout> layout);
        ~PipelineBase();

        auto CreateCache() -> void;
        auto CreateShaderModule(const schematic::ShaderInfo &shader, VkShaderModule &shaderModule) const -> void;
        [[nodiscard]] auto CreateShaderStageInfo(
            const VkShaderStageFlagBits stage,
            const VkShaderModule &shaderModule,
            const std::string_view entryPoint
        ) const -> VkPipelineShaderStageCreateInfo;

    protected:
        const DeviceContext &m_device;
        std::shared_ptr<PipelineLayout> m_layout{nullptr};
        VkPipelineCache m_cache{nullptr};
        std::vector<VkShaderModule> m_shaders{};
        VkPipeline m_pipeline{nullptr};
    };

}
