#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    class DescriptorPool2;
    class DescriptorSetLayout2;

    class DescriptorWriter2 {
    public:
        DescriptorWriter2(DescriptorPool2 &pool, DescriptorSetLayout2 &setLayout);

        DescriptorWriter2 &addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *bufferInfo);
        DescriptorWriter2 &addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *imageInfo);

        void writeAll(vk::DescriptorSet set);

    private:
        DescriptorPool2 &pool;
        DescriptorSetLayout2 &setLayout;
        std::vector<vk::WriteDescriptorSet> writes;
    };

}
