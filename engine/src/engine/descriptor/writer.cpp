#include "muon/engine/descriptor.hpp"
#include "muon/engine/descriptor/writer.hpp"

#include "muon/engine/descriptor/pool.hpp"
#include "muon/engine/descriptor/setlayout.hpp"

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

    /* NEW API */

    DescriptorWriter2::DescriptorWriter2(
        DescriptorPool2 &pool,
        DescriptorSetLayout2 &setLayout
    ) : pool(pool), setLayout(setLayout) {}

    DescriptorWriter2 &DescriptorWriter2::addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *bufferInfo) {
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

    DescriptorWriter2 &DescriptorWriter2::addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *imageInfo) {
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

    void DescriptorWriter2::writeAll(vk::DescriptorSet set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        pool.device.getDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
    }

}
