#pragma once

#include "muon/graphics/pipeline_layout.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <filesystem>
#include <map>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineGraphicsState {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizationState{};
        VkPipelineMultisampleStateCreateInfo multisampleState{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        VkPipelineDepthStencilStateCreateInfo depthStencilState{};
        std::vector<VkDynamicState> dynamicStateEnables{};
        VkPipelineDynamicStateCreateInfo dynamicState{};

        PipelineGraphicsState();
    };

    struct PipelineGraphicsSpecification {
        VkDevice device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        PipelineGraphicsState state{};
        std::map<VkShaderStageFlags, std::filesystem::path> paths{};
    };

    class PipelineGraphics : NoCopy, NoMove {
    public:
        PipelineGraphics(const PipelineGraphicsSpecification &spec);
        ~PipelineGraphics();

    private:
        void CreateCache();
        void CreateShaderModules(const std::map<VkShaderStageFlags, std::filesystem::path> &paths);
        void CreatePipeline(const VkPipelineRenderingCreateInfo &renderingCreateInfo);

    private:
        VkDevice m_device{nullptr};

        std::shared_ptr<PipelineLayout> m_layout{nullptr};
        VkPipelineCache m_cache{nullptr};

        std::vector<VkShaderModule> m_shaders{};
        std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages{};
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions{};
        std::optional<VkVertexInputBindingDescription> m_bindingDescription{std::nullopt};

        VkPipeline m_pipeline{nullptr};
    };

}
