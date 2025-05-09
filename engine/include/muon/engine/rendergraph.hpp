#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class RenderGraph {
    public:
        struct Resource;
        struct ResourceUsage;
        enum class StageType;
        struct Stage;
        struct FrameInfo;

        void addResource(Resource resource);
        void addStage(Stage stage);

        void compile();
        void execute(vk::CommandBuffer commandBuffer);

    private:
        std::unordered_map<std::string, Resource> resources{};

        std::vector<std::shared_ptr<Stage>> stages;
        bool stagesUpdated{false};
        std::unordered_map<std::string, std::vector<std::string>> dependencies;

        std::vector<std::shared_ptr<Stage>> executionOrder;
        std::vector<std::shared_ptr<Stage>> determineExecutionOrder();
    };

    struct RenderGraph::Resource {
        /* to be filled out */
    };

    struct RenderGraph::ResourceUsage {
        std::string name;

        vk::ImageLayout layout;
        vk::AccessFlags accessFlags;
        vk::PipelineStageFlags stageFlags;
    };

    enum class RenderGraph::StageType {
        Graphics,
        Compute,
        Transfer,
    };

    struct RenderGraph::Stage {
        std::string name;
        StageType stageType;

        std::vector<ResourceUsage> readResources{};
        std::vector<ResourceUsage> writeResources{};

        std::function<void()> compile;
        std::function<void(vk::CommandBuffer, FrameInfo)> execute;
    };

    struct RenderGraph::FrameInfo {
        uint32_t frameIndex;
        uint32_t pingPongIndex{0};
    };

}
