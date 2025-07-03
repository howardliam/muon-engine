#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/pipeline_info.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <glm/vec3.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineComputeSpecification {
        const DeviceContext *device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        schematic::PipelineInfo pipelineInfo{};
    };

    class PipelineCompute : NoCopy, NoMove {
    public:
        PipelineCompute(const PipelineComputeSpecification &spec);
        ~PipelineCompute();

        void Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets) const;
        void Dispatch(VkCommandBuffer cmd, const VkExtent2D &extent, const glm::uvec3 &groupCount) const;

    public:
        [[nodiscard]] VkPipeline Get() const;

    private:
        void CreateCache();
        void CreateShaderModule(const schematic::ShaderInfo &shader);
        void CreatePipeline();

    private:
        const DeviceContext &m_device;

        std::shared_ptr<PipelineLayout> m_layout{nullptr};
        VkPipelineCache m_cache{nullptr};

        VkShaderModule m_shader{nullptr};

        VkPipeline m_pipeline{nullptr};
    };

}
