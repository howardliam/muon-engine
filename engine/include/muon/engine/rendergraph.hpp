#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace muon::engine {
    class Device;
    class Image;
}

namespace muon::engine::rg {

    enum class ResourceOp {
        Read,
        Write,
    };

    struct ResourceConfig {
        vk::Format format;
        vk::ImageLayout layout;
        vk::ImageUsageFlags usageFlags;
        vk::AccessFlags2 accessFlags;
        vk::PipelineStageFlags2 stageFlags;
    };

    struct ResourceUsage {
        std::string name;
        ResourceOp op;
        vk::ImageLayout layout;
        vk::AccessFlags2 accessFlags;
        vk::PipelineStageFlags2 stageFlags;
    };

    enum class NodeType {
        Render,
        PostProcessing,
        Transfer,
        Presentation,
    };

    struct Node {
        std::string name;
        NodeType type;

        std::vector<ResourceUsage> resources{};

        std::vector<vk::ImageMemoryBarrier2> preBarriers{};
        std::vector<vk::ImageMemoryBarrier2> postBarriers{};
    };

    class RenderGraph {
    public:
        RenderGraph(Device &device);
        ~RenderGraph();

        void setImageSize(const vk::Extent2D &imageSize);

        void addResource(const std::string &name, const ResourceConfig &resource);

        void addNode(const Node &node);

        void compile();

        void execute();

    private:
        Device &device;

        vk::Extent2D imageSize;

        std::unordered_map<std::string, ResourceConfig> resourceBlueprints;
        std::unordered_map<std::string, std::unique_ptr<Image>> resources;  // is populated upon compilation

        std::vector<Node> nodes;

    };

}
