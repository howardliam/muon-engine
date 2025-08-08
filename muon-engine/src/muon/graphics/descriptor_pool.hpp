#pragma once

#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <vector>

namespace muon::graphics {

class DescriptorPool {
public:
    DescriptorPool(
        const Context &context,
        uint32_t maxSets,
        const std::vector<vk::DescriptorPoolSize> &poolSizes
    );
    ~DescriptorPool();

public:
    auto get() -> vk::raii::DescriptorPool &;
    auto get() const -> const vk::raii::DescriptorPool &;

private:
    const Context &m_context;

    vk::raii::DescriptorPool m_pool{nullptr};
};

} // namespace muon::graphics
