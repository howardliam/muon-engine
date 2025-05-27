#include "muon/renderer/descriptor/writer.hpp"

#include "muon/renderer/descriptor/pool.hpp"
#include "muon/renderer/descriptor/set_layout.hpp"
#include "muon/renderer/device.hpp"

namespace muon {

    DescriptorWriter::DescriptorWriter(
        DescriptorPool &pool,
        DescriptorSetLayout &setLayout
    ) : m_pool(pool), m_setLayout(setLayout) {}

    DescriptorWriter &DescriptorWriter::addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *bufferInfo) {
        auto &bindingDescription = m_setLayout.m_bindings[binding];

        vk::WriteDescriptorSet write{};
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;
        write.dstArrayElement = position;

        m_writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *imageInfo) {
        auto &bindingDescription = m_setLayout.m_bindings[binding];

        vk::WriteDescriptorSet write{};
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;
        write.dstArrayElement = position;

        m_writes.push_back(write);
        return *this;
    }

    void DescriptorWriter::writeAll(vk::DescriptorSet set) {
        for (auto &write : m_writes) {
            write.dstSet = set;
        }
        m_pool.m_device.device().updateDescriptorSets(m_writes.size(), m_writes.data(), 0, nullptr);

        m_writes.clear();
    }

}
