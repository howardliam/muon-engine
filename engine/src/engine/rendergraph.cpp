#include "muon/engine/rendergraph.hpp"

#include <algorithm>
#include <queue>

namespace muon::engine {

    void RenderGraph::addImage(const std::string &name, std::unique_ptr<Image> image) {
        resources[name] = std::move(image);
        resourceAliases[name] = name;
    }

    void RenderGraph::addAlias(const std::string &alias, const std::string &name) {
        if (!resources.contains(name)) {
            return;
        }

        resourceAliases[alias] = name;
    }

    void RenderGraph::addNode(Node node) {
        nodes.push_back(std::make_shared<Node>(std::move(node)));
        nodesUpdated = true;
    }

    void RenderGraph::compile() {
        for (const auto &node : nodes) {
            auto &deps = dependencies[node->name];

            auto &reads = node->readResources;
            for (const auto &read : reads) {
                for (const auto &otherNode : nodes) {
                    if (otherNode->name == node->name) {
                        continue;
                    }

                    auto it = std::find_if(
                        otherNode->writeResources.begin(),
                        otherNode->writeResources.end(),
                        [&read](const ResourceUsage &usage) -> bool {
                            return usage.name == read.name;
                        }
                    );

                    if (it != otherNode->writeResources.end()) {
                        if (std::find(deps.begin(), deps.end(), otherNode->name) == deps.end()) {
                            deps.push_back(otherNode->name);
                        }
                    }
                }
            }
        }
    }

    void RenderGraph::execute(vk::CommandBuffer commandBuffer) {
        if (nodesUpdated) {
            executionOrder = determineExecutionOrder();
            nodesUpdated = false;
        }

        for (auto &node : executionOrder) {
            node->execute(commandBuffer);
        }
    }

    Image *RenderGraph::getImage(const std::string &name) const {
        auto aliasesIt = resourceAliases.find(name);
        if (aliasesIt == resourceAliases.end()) {
            return nullptr;
        }

        auto resourcesIt = resources.find(aliasesIt->second);
        if (resourcesIt == resources.end()) {
            return nullptr;
        }
        return resourcesIt->second.get();
    }

    std::vector<std::shared_ptr<RenderGraph::Node>> RenderGraph::determineExecutionOrder() {
        std::unordered_map<std::string, std::shared_ptr<Node>> nameToNode;
        for (auto &node : nodes) {
            nameToNode[node->name] = node;
        }

        std::unordered_map<std::string, int32_t> inDegree;
        for (auto &[name, deps] : dependencies) {
            inDegree[name] = deps.size();
        }

        std::queue<std::string> queue;
        for (auto &[name, degree] : inDegree) {
            if (degree == 0) {
                queue.push(name);
            }
        }

        std::vector<std::shared_ptr<Node>> executionOrder;

        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop();
            executionOrder.push_back(nameToNode[current]);

            for (auto &[dependent, deps] : dependencies) {
                auto it = std::find(deps.begin(), deps.end(), current);
                if (it != deps.end()) {
                    inDegree[dependent] -= 1;
                    if (inDegree[dependent] == 0) {
                        queue.push(dependent);
                    }
                }
            }
        }

        return executionOrder;
    }
}
