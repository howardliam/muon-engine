#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class DescriptorPool;
    class DescriptorSetLayout;

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorPool &pool, DescriptorSetLayout &setLayout);

        DescriptorWriter &addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *bufferInfo);
        DescriptorWriter &addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *imageInfo);

        void writeAll(vk::DescriptorSet set);

    private:
        DescriptorPool &pool;
        DescriptorSetLayout &setLayout;
        std::vector<vk::WriteDescriptorSet> writes;
    };

}
