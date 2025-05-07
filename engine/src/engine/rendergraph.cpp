#include "muon/engine/rendergraph.hpp"

#include "muon/log/logger.hpp"
#include <algorithm>
#include <queue>

namespace muon::engine {

    void RenderGraph::addResource(Resource resource) {

    }

    void RenderGraph::addStage(Stage stage) {
        stages.push_back(std::make_shared<Stage>(std::move(stage)));
    }

    void RenderGraph::compile() {
        for (const auto &stage : stages) {
            auto &deps = dependencies[stage->name];

            auto &reads = stage->readResources;
            for (const auto &read : reads) {
                for (const auto &otherStage : stages) {
                    if (otherStage->name == stage->name) {
                        continue;
                    }

                    auto it = std::find_if(
                        otherStage->writeResources.begin(),
                        otherStage->writeResources.end(),
                        [&read](const ResourceUsage &usage) -> bool {
                            return usage.name == read.name;
                        }
                    );

                    if (it != otherStage->writeResources.end()) {
                        if (std::find(deps.begin(), deps.end(), otherStage->name) == deps.end()) {
                            deps.push_back(otherStage->name);
                        }
                    }
                }
            }
        }
    }

    void RenderGraph::execute(vk::CommandBuffer commandBuffer) {
        auto order = executionOrder();

        for (auto &stage : order) {
            stage->execute(commandBuffer);
        }
    }

    std::vector<std::shared_ptr<RenderGraph::Stage>> RenderGraph::executionOrder() {
        std::unordered_map<std::string, std::shared_ptr<Stage>> nameToStage;
        for (auto &stage : stages) {
            nameToStage[stage->name] = stage;
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

        std::vector<std::shared_ptr<Stage>> executionOrder;

        while (!queue.empty()) {
            std::string current = queue.front();
            queue.pop();
            executionOrder.push_back(nameToStage[current]);

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
