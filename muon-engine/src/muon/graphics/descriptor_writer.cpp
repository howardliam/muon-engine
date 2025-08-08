#include "muon/graphics/descriptor_writer.hpp"

#include "muon/core/log.hpp"
#include "muon/graphics/descriptor_set_layout.hpp"

namespace muon::graphics {

DescriptorWriter::DescriptorWriter(
    const Context &context,
    DescriptorPool &pool,
    DescriptorSetLayout &setLayout
) : m_context{context}, m_pool{pool}, m_setLayout{setLayout} {
    core::debug("created descriptor writer");
}

DescriptorWriter::~DescriptorWriter() { core::debug("destroyed descriptor writer"); }

auto DescriptorWriter::addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *info) -> DescriptorWriter & {
    auto &bindingDescription = m_setLayout.getBindings()[binding];

    vk::WriteDescriptorSet write;
    write.descriptorCount = 1;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.dstArrayElement = position;
    write.pBufferInfo = info;

    m_writes.push_back(write);
    return *this;
}

auto DescriptorWriter::addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *info) -> DescriptorWriter & {
    auto &bindingDescription = m_setLayout.getBindings()[binding];

    vk::WriteDescriptorSet write;
    write.descriptorCount = 1;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.dstArrayElement = position;
    write.pImageInfo = info;

    m_writes.push_back(write);
    return *this;
}

auto DescriptorWriter::writeAll(vk::raii::DescriptorSet &set) -> void {
    for (auto &write : m_writes) {
        write.dstSet = set;
    }

    m_context.getDevice().updateDescriptorSets(m_writes, {});
    m_writes.clear();
}

} // namespace muon::graphics
