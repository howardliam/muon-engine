#include "muon/engine/rendergraph.hpp"

#include "muon/log/logger.hpp"
#include <algorithm>
#include <ranges>
#include <iostream>

namespace muon::engine {

    void RenderGraph::addResource(Resource resource) {

    }

    void RenderGraph::addStage(Stage stage) {
        stages.push_back(std::make_shared<Stage>(std::move(stage)));
    }

    void RenderGraph::compile() {
        for (auto &stage : stages) {
            dependencies[stage->name];

            auto &reads = stage->readResources;

            for (auto &depStage : stages) {
                for (auto &read : reads) {

                    auto it = std::find_if(
                        depStage->writeResources.begin(),
                        depStage->writeResources.end(),
                        [&read](const ResourceUsage& usage) -> bool {
                            return usage.name == read.name;
                        }
                    );

                    if (it != depStage->writeResources.end()) {
                        dependencies[stage->name].push_back(depStage->name);
                    }
                }
            }
        }

        for (auto &[dependent, dependency] : dependencies) {
            log::globalLogger->info("Stage {} depends on stages: {}", dependent, dependency);
        }
    }

    void RenderGraph::execute(vk::CommandBuffer commandBuffer) {

    }

}
