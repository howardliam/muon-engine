#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace {
    using DependencyMap = std::unordered_map<std::string, std::unordered_set<std::string>>;
}

namespace muon::engine {
    class Device;
    class Image;
    class DescriptorPool;
    class DescriptorSetLayout;
}

namespace muon::engine::rg {

    struct Node {
        std::string name;
        std::vector<std::string> readDeps;
        std::vector<std::string> writeDeps;
    };

    class DescriptorHelper {
    public:
        struct Write {
            uint32_t binding;
            uint32_t position;
            std::string name;
        };

        struct WriteContext {
            DescriptorPool *pool;
            DescriptorSetLayout *layout;
            vk::DescriptorSet set;
            std::vector<Write> writes;
        };

        DescriptorHelper(DescriptorPool *pool, DescriptorSetLayout *layout, vk::DescriptorSet set);

        DescriptorHelper &write(uint32_t binding, uint32_t position, const std::string &name);

    private:
        WriteContext writeContext;

        WriteContext getContext();

        friend class RenderGraph;
    };

    class ResourceBuilder {
    public:
        struct Layout {
            vk::Extent2D extent;
            vk::Format format;
            vk::ImageUsageFlags usageFlags;
            vk::ImageLayout layout;
            vk::AccessFlags2 accessFlags;
            vk::PipelineStageFlags2 stageFlags;
        };

        ResourceBuilder();

        void addImage(const std::string &name, const Layout &layout);

        DescriptorHelper &writeDescriptors(DescriptorPool *pool, DescriptorSetLayout *layout, vk::DescriptorSet set);

    private:
        std::unordered_map<std::string, Layout> images;
        std::vector<DescriptorHelper> descriptorHelpers;

        friend class RenderGraph;
    };

    class RenderGraph {
    public:
        RenderGraph(Device &device);
        ~RenderGraph();

        void configureResources(std::function<void(ResourceBuilder &)> callback);

        void addNode(Node node);

        void compile();
        void execute();

    private:
        Device &device;

        std::unordered_map<std::string, std::unique_ptr<Image>> images;

        std::unordered_map<std::string, Node> nodes;
        std::vector<std::string> order;

        DependencyMap determineDependencies(const std::unordered_map<std::string, Node> &nodes);

        std::vector<std::string> topographicalSort(const DependencyMap &dependencies);
    };

}
