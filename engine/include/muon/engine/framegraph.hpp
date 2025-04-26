#pragma once

#include <functional>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    struct FrameGraphStage {

        std::function<void(vk::CommandBuffer)> record;
    };

    class FrameGraph {
    public:
        FrameGraph();
        ~FrameGraph();

        void addStage(FrameGraphStage stage);
        void compile();
        void execute(vk::CommandBuffer commandBuffer);

    private:


    };

}
