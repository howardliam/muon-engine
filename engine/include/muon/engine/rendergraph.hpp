#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

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

    };

}
