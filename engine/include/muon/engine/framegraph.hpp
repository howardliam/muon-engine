#pragma once

#include <functional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class FrameGraph {
    public:
        struct Resource {
            std::string name;
        };

        struct Stage {
            std::string name;
            std::vector<std::string> readResources;
            std::vector<std::string> writeResources;

            std::function<void()> compile;

            std::function<void(vk::CommandBuffer)> record;
        };

        FrameGraph() = default;
        ~FrameGraph() = default;

        void addResource(Resource resource);

        void addStage(Stage stage);

        void compile();

        void execute(vk::CommandBuffer commandBuffer);

    private:
        std::unordered_map<std::string, Resource> resources{};
        std::vector<Stage> stages{};

    };

}
