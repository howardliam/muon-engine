#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace {
    using DependencyMap = std::unordered_map<std::string, std::unordered_set<std::string>>;
}

// namespace muon::engine {
//     class Device;
//     class Image;
// }

namespace muon::engine::rg {

    struct Node {
        std::string name;
        std::vector<std::string> readDeps;
        std::vector<std::string> writeDeps;
    };

    class RenderGraph {
    public:
        RenderGraph();
        ~RenderGraph();

        void addNode(Node node);

        void compile();
        void execute();

    private:
        std::unordered_map<std::string, Node> nodes;
        std::vector<std::string> order;

        DependencyMap determineDependencies(const std::unordered_map<std::string, Node> &nodes);

        std::vector<std::string> topographicalSort(const DependencyMap &dependencies);
    };

}
