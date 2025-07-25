#include "muon/graphics/descriptor_pool.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

namespace muon::graphics {

DescriptorPool::DescriptorPool(const Spec &spec) : m_context{*spec.context} {
    vk::DescriptorPoolCreateInfo descriptorPoolCi;
    descriptorPoolCi.maxSets = spec.maxSets;
    descriptorPoolCi.poolSizeCount = spec.poolSizes.size();
    descriptorPoolCi.pPoolSizes = spec.poolSizes.data();
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
