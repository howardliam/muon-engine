#include "muon/engine/rendergraph.hpp"

#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <algorithm>

#include "muon/log/logger.hpp"
#include "muon/engine/device.hpp"
#include "muon/engine/image.hpp"

namespace muon::engine::rg {

    DescriptorHelper::DescriptorHelper(DescriptorPool *pool, DescriptorSetLayout *layout, vk::DescriptorSet set) {
        writeContext.pool = pool;
        writeContext.layout = layout;
        writeContext.set = set;
    }

    DescriptorHelper &DescriptorHelper::write(uint32_t binding, uint32_t position, const std::string &name) {
        writeContext.writes.emplace_back(Write{ binding, position, name });
        return *this;
    }

    DescriptorHelper::WriteContext DescriptorHelper::getContext() {
        return writeContext;
    }

    ResourceBuilder::ResourceBuilder() {}

    void ResourceBuilder::addImage(const std::string &name, const Layout &layout) {
        images[name] = layout;
    }

    DescriptorHelper &ResourceBuilder::writeDescriptors(DescriptorPool *pool, DescriptorSetLayout *layout, vk::DescriptorSet set) {
        descriptorHelpers.emplace_back(pool, layout, set);
        return descriptorHelpers.back();
    }

    RenderGraph::RenderGraph(Device &device) : device(device) {}

    RenderGraph::~RenderGraph() {}

    void RenderGraph::configureResources(std::function<void(ResourceBuilder &)> callback) {
        ResourceBuilder builder;
        callback(builder);

        for (const auto &[name, image] : builder.images) {
            log::globalLogger->info("image: {}", name);
        }

        for (const auto &helper : builder.descriptorHelpers) {
            for (const auto &write : helper.writeContext.writes) {
                log::globalLogger->info("writes: {} into binding: {} at position: {}", write.name, write.binding, write.position);
            }
        }
    }

    void RenderGraph::addNode(Node node) {
        nodes[node.name] = node;
    }

    void RenderGraph::compile() {
        auto dependencies = determineDependencies(nodes);
        order = topographicalSort(dependencies);

        for (const auto &node : order) {
            log::globalLogger->info("node: {}", node);
        }
    }

    void RenderGraph::execute() {

    }

    DependencyMap RenderGraph::determineDependencies(const std::unordered_map<std::string, Node> &nodes) {
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

    std::vector<std::string> RenderGraph::topographicalSort(const DependencyMap &dependencies) {
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
