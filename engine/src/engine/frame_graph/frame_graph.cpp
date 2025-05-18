#include "muon/engine/frame_graph/frame_graph.hpp"

#include "muon/engine/image.hpp"
#include <queue>

namespace muon::engine::fg {

    FrameGraph::FrameGraph(Device &device) : device(device) {

    }

    FrameGraph::~FrameGraph() {

    }

    DependencyMap FrameGraph::determineDependencies(const std::unordered_map<std::string, Node> &nodes) {
        DependencyMap dependencies;

        for (const auto &[_, node] : nodes) {
            dependencies[node.name];

            for (const auto &[_, other] : nodes) {
                if (node.name == other.name) { continue; }

                auto &readDeps = node.readDeps;
                for (auto &readDep : readDeps) {
                    auto predicate = [&readDep](const auto &writeDep) { return readDep == writeDep; };
                    auto it = std::find_if(other.writeDeps.begin(), other.writeDeps.end(), predicate);

                    if (it == other.writeDeps.end()) { continue; }

                    dependencies[node.name].insert(other.name);
                }
            }
        }

        return dependencies;
    }

    std::vector<std::string> FrameGraph::topographicalSort(const DependencyMap &dependencies) {
        std::unordered_map<std::string, uint32_t> inDegrees;
        std::queue<std::string> queue;

        for (const auto &[node, deps] : dependencies) {
            inDegrees[node] = deps.size();
            if (inDegrees[node] == 0) { queue.push(node); }
        }

        size_t index{0};
        std::vector<std::string> order(dependencies.size());

        while (!queue.empty()) {
            if (index >= order.size()) {
                throw std::runtime_error("cycle detected");
            }

            auto node = queue.front();
            queue.pop();

            order[index++] = node;

            for (auto &[dep, deps] : dependencies) {
                if (!deps.contains(node)) { continue; }

                inDegrees[dep] -= 1;
                if (inDegrees[dep] == 0) { queue.push(dep); }
            }
        }

        return order;
    }

}
