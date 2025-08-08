#include "muon/graphics/descriptor_pool.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

namespace muon::graphics {

DescriptorPool::DescriptorPool(
    const Context &context,
    uint32_t maxSets,
    const std::vector<vk::DescriptorPoolSize> &poolSizes
) : m_context{context} {
    vk::DescriptorPoolCreateInfo descriptorPoolCi;
    descriptorPoolCi.maxSets = maxSets;
    descriptorPoolCi.poolSizeCount = poolSizes.size();
    descriptorPoolCi.pPoolSizes = poolSizes.data();
    descriptorPoolCi.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;

    auto descriptorPoolResult = m_context.getDevice().createDescriptorPool(descriptorPoolCi);
    core::expect(descriptorPoolResult, "failed to create descriptor pool");

    m_pool = std::move(*descriptorPoolResult);

    core::debug("created descriptor pool");
}

DescriptorPool::~DescriptorPool() { core::debug("destroyed descriptor pool"); }

auto DescriptorPool::get() -> vk::raii::DescriptorPool & { return m_pool; }
auto DescriptorPool::get() const -> const vk::raii::DescriptorPool & { return m_pool; }

} // namespace muon::graphics
