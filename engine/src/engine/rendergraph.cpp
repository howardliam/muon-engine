#include "muon/engine/rendergraph.hpp"

#include "muon/log/logger.hpp"
#include "muon/engine/device.hpp"
#include "muon/engine/image.hpp"

namespace muon::engine::rg {

    RenderGraph::RenderGraph(Device &device) : device(device) {}

    RenderGraph::~RenderGraph() {}

    void RenderGraph::setImageSize(const vk::Extent2D &imageSize) {
        this->imageSize = imageSize;
    }

    void RenderGraph::addResource(const std::string &name, const ResourceConfig &resource) {
        resourceBlueprints[name] = resource;
    }

    void RenderGraph::addNode(const Node &node) {
        nodes.push_back(node);
    }

    void RenderGraph::compile() {
        for (const auto &[name, resource] : resourceBlueprints) {
            log::globalLogger->info("creating image {}", name);
            resources[name] = engine::Image::Builder(device)
                .setExtent(imageSize)
                .setFormat(resource.format)
                .setImageLayout(resource.layout)
                .setImageUsageFlags(resource.usageFlags)
                .setAccessFlags(resource.accessFlags)
                .setPipelineStageFlags(resource.stageFlags)
                .buildUniquePtr();
        }
    }

}
