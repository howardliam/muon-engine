#include "muon/engine/descriptor/writer.hpp"

#include "muon/engine/descriptor/pool.hpp"
#include "muon/engine/descriptor/setlayout.hpp"
#include "muon/engine/device.hpp"

namespace muon::engine {

    DescriptorWriter::DescriptorWriter(
        DescriptorPool &pool,
        DescriptorSetLayout &setLayout
    ) : pool(pool), setLayout(setLayout) {}

    DescriptorWriter &DescriptorWriter::addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *bufferInfo) {
        auto &bindingDescription = setLayout.bindings[binding];

        vk::WriteDescriptorSet write{};
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;
        write.dstArrayElement = position;

        writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *imageInfo) {
        auto &bindingDescription = setLayout.bindings[binding];

        vk::WriteDescriptorSet write{};
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;
        write.dstArrayElement = position;

        writes.push_back(write);
        return *this;
    }

    void DescriptorWriter::writeAll(vk::DescriptorSet set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        pool.device.getDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
    }

}
