#include "muon/engine/rendergraph.hpp"

#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>

#include "muon/log/logger.hpp"
#include "muon/engine/device.hpp"
#include "muon/engine/image.hpp"

namespace muon::engine::rg {

    RenderGraph::RenderGraph()  {}

    RenderGraph::~RenderGraph() {}

    void RenderGraph::addNode(Node node) {
        nodes[node.name] = node;
    }

    void RenderGraph::compile() {

        std::unordered_map<std::string, std::unordered_set<std::string>> dependencies;

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

        std::unordered_map<std::string, uint32_t> inDegrees;
        std::queue<std::string> queue;

        for (const auto &[node, deps] : dependencies) {
            inDegrees[node] = deps.size();
            if (inDegrees[node] == 0) { queue.push(node); }
        }

        std::vector<std::string> order;

        while (!queue.empty()) {
            auto node = queue.front();
            queue.pop();
            order.push_back(node);

            for (auto &[dep, deps] : dependencies) {
                if (!deps.contains(node)) { continue; }

                inDegrees[dep] -= 1;
                if (inDegrees[dep] == 0) { queue.push(dep); }
            }
        }
    }

    void RenderGraph::execute() {

    }

}
