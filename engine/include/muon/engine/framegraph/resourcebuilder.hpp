#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace muon::engine {
    class DescriptorPool;
    class DescriptorSetLayout;
}

namespace muon::engine::fg {

    class DescriptorHelper;

    struct ResourceLayout {
        vk::Extent2D extent;
        vk::Format format;
        vk::ImageUsageFlags usageFlags;
        vk::ImageLayout layout;
        vk::AccessFlags2 accessFlags;
        vk::PipelineStageFlags2 stageFlags;
    };

    class ResourceBuilder {
    public:
        ResourceBuilder();

        void addImage(const std::string &name, const ResourceLayout &layout);

        DescriptorHelper &writeDescriptors(DescriptorPool *pool, DescriptorSetLayout *layout, vk::DescriptorSet set);

    private:
        std::unordered_map<std::string, ResourceLayout> images;
        std::vector<DescriptorHelper> descriptorHelpers;

        friend class FrameGraph;
    };

}
