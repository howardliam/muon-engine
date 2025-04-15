#include "muon/engine/descriptor.hpp"

namespace muon::engine {

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool) : setLayout(setLayout), pool(pool) {}

    DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo) {
        auto &bindingDescription = setLayout.bindings[binding];

        vk::WriteDescriptorSet write{};
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo) {
        auto &bindingDescription = setLayout.bindings[binding];

        vk::WriteDescriptorSet write{};
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::build(vk::DescriptorSet &set) {
        bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(vk::DescriptorSet &set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        pool.device.getDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
    }

}
