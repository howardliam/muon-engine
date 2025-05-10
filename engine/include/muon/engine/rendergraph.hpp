#pragma once

#include "muon/engine/image.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class RenderGraph {
    public:
        struct ResourceUsage;
        enum class NodeType;
        struct Node;
        struct FrameInfo;

        void addImage(const std::string &name, std::unique_ptr<Image> image);
        void addNode(Node node);

        void compile();
        void execute(vk::CommandBuffer commandBuffer);

    private:
        std::unordered_map<std::string, std::unique_ptr<Image>> resources{};

        std::vector<std::shared_ptr<Node>> nodes;
        bool nodesUpdated{false};
        std::unordered_map<std::string, std::vector<std::string>> dependencies;

        std::vector<std::shared_ptr<Node>> executionOrder;
        std::vector<std::shared_ptr<Node>> determineExecutionOrder();
    };

    struct RenderGraph::ResourceUsage {
        std::string name;

        vk::ImageLayout layout;
        vk::AccessFlags accessFlags;
        vk::PipelineStageFlags stageFlags;
    };

    enum class RenderGraph::NodeType {
        Graphics,
        Compute,
        Transfer,
    };

    struct RenderGraph::Node {
        std::string name;
        NodeType nodeType;

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
