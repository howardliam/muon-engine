#pragma once

#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstddef>
#include <cstdint>

namespace muon::graphics {

class DescriptorPool;
class DescriptorSetLayout;

class DescriptorWriter {
public:
    struct Spec {
        const Context &context;
        DescriptorPool &pool;
        DescriptorSetLayout &setLayout;

        Spec(const Context &context, DescriptorPool &pool, DescriptorSetLayout &setLayout)
            : context{context}, pool{pool}, setLayout{setLayout} {}
    };

public:
    DescriptorWriter(const Spec &spec);
    ~DescriptorWriter();

    auto addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *info) -> DescriptorWriter &;
    auto addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *info) -> DescriptorWriter &;

    auto writeAll(vk::raii::DescriptorSet &set) -> void;

private:
    const Context &m_context;

    DescriptorPool &m_pool;
    DescriptorSetLayout &m_setLayout;

    std::vector<vk::WriteDescriptorSet> m_writes;
};

} // namespace muon::graphics
