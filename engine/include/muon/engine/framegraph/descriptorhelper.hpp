#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon::engine {
    class DescriptorPool;
    class DescriptorSetLayout;
}

namespace muon::engine::fg {

    class DescriptorHelper {
    public:
        struct Write {
            uint32_t binding;
            uint32_t position;
            std::string name;
        };

        struct WriteContext {
            DescriptorPool *pool;
            DescriptorSetLayout *layout;
            vk::DescriptorSet set;
            std::vector<Write> writes;
        };

        DescriptorHelper(
            DescriptorPool *pool,
            DescriptorSetLayout *layout,
            vk::DescriptorSet set
        );

        DescriptorHelper &write(uint32_t binding, uint32_t position, const std::string &name);

    private:
        DescriptorPool *pool;
        DescriptorSetLayout *layout;
        vk::DescriptorSet set;
        std::vector<Write> writes;

        friend class FrameGraph;
    };

}
