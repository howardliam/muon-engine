#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace {
    using DependencyMap = std::unordered_map<std::string, std::unordered_set<std::string>>;
}

namespace muon::engine {
    class Device;
    class Image;
}

namespace muon::engine::fg {

    class ResourceBuilder;

    struct Node {
        std::string name;
        std::vector<std::string> readDeps;
        std::vector<std::string> writeDeps;
    };

    class FrameGraph : NoCopy, NoMove {
    public:
        class Builder;

        FrameGraph(Device &device);
        ~FrameGraph();

        void configureResources(std::function<void(Builder &)> callback);

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

    class FrameGraph::Builder : NoCopy, NoMove {
    public:

        void addImage(const std::string &name);

    private:



        friend class FrameGraph;
    };

}
