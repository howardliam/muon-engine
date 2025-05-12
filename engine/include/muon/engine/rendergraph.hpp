#pragma once

#include "muon/engine/image.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    class RenderGraph {
    public:
        struct ResourceUsage;
        enum class NodeType;
        struct Node;
        struct FrameInfo;

        void addImage(const std::string &name, std::unique_ptr<Image> image);

        void addAlias(const std::string &alias, const std::string &name);

        void addNode(Node node);

        void compile();

        void execute(vk::CommandBuffer commandBuffer);

        Image *getImage(const std::string &name) const;

    private:
        std::unordered_map<std::string, std::unique_ptr<Image>> resources{};
        std::unordered_map<std::string, std::string> resourceAliases{};

        std::vector<std::shared_ptr<Node>> nodes;
        bool nodesUpdated{false};
        std::unordered_map<std::string, std::vector<std::string>> dependencies;

        std::unordered_map<std::string, vk::DependencyInfo> preNodeTransitions{};

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
        std::function<void(vk::CommandBuffer)> execute;
    };

    struct RenderGraph::FrameInfo {
        uint32_t frameIndex;
        uint32_t pingPongIndex{0};
    };

}
