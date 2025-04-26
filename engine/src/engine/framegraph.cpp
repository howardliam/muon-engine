#include "muon/engine/framegraph.hpp"

namespace muon::engine {

    void FrameGraph::addResource(Resource resource) {
        resources[resource.name] = resource;
    }

    void FrameGraph::addStage(Stage stage) {
        stages.push_back(stage);
    }

    void FrameGraph::compile() {
        for (auto &stage : stages) {
            stage.compile();
        }
    }

    void FrameGraph::execute(vk::CommandBuffer commandBuffer) {
        for (auto &stage : stages) {
            stage.record(commandBuffer);
        }
    }

}
