#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <functional>
#include <vulkan/vulkan_enums.hpp>

namespace muon::engine {

    struct RenderResource {
        /* to be filled out */
    };

    enum class StageType {
        Graphics,
        Compute,
        Transfer,
    };

    struct RenderResourceUsage {
        std::string name;

        vk::ImageLayout layout;
        vk::AccessFlags accessFlags;
        vk::PipelineStageFlags stageFlags;
    };

    struct RenderStage {
        std::string name;
        StageType stageType;

        std::vector<RenderResourceUsage> readResources{};
        std::vector<RenderResourceUsage> writeResources{};

        std::function<void()> compile;
        std::function<void(vk::CommandBuffer)> execute;
    };

    class RenderGraph {
    public:
        void addResource(RenderResource resource);
        void addStage(RenderStage stage);

        void compile();
        void execute(vk::CommandBuffer commandBuffer);

    private:
        std::unordered_map<std::string, RenderResource> resources{};
        std::vector<RenderStage> stages{};
    };

}
