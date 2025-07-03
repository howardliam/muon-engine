#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/graphics/pipeline_layout.hpp"
#include "muon/schematic/pipeline/pipeline_info.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineMeshletSpecification {
        const DeviceContext *device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        schematic::PipelineInfo pipelineInfo{};
    };

    class PipelineMeshlet : NoCopy, NoMove {
    public:
        PipelineMeshlet(const PipelineMeshletSpecification &spec);
        ~PipelineMeshlet();

        void Bake(const VkPipelineRenderingCreateInfo &renderingCreateInfo);
        void Bind(VkCommandBuffer cmd, const std::vector<VkDescriptorSet> &sets);

    private:
        void CreateCache();
        void CreateShaderModules(const std::unordered_map<schematic::ShaderStage, schematic::ShaderInfo> &shaders);
        void CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo);

    private:
        const DeviceContext &m_device;

        std::shared_ptr<PipelineLayout> m_layout{nullptr};
        VkPipelineCache m_cache{nullptr};

        struct PipelineMeshletState {
            VkPipelineViewportStateCreateInfo viewportState{};
            VkPipelineRasterizationStateCreateInfo rasterizationState{};
            VkPipelineMultisampleStateCreateInfo multisampleState{};
            std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
            VkPipelineColorBlendStateCreateInfo colorBlendState{};
            VkPipelineDepthStencilStateCreateInfo depthStencilState{};
            std::vector<VkDynamicState> dynamicStateEnables{};
            VkPipelineDynamicStateCreateInfo dynamicState{};
        };
        PipelineMeshletState m_state{};

        std::vector<VkShaderModule> m_shaders{};
        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};

        VkPipeline m_pipeline{nullptr};
    };

}
