#include "muon/graphics/descriptor_set_layout.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

#include <vector>

namespace muon::graphics {

DescriptorSetLayout::DescriptorSetLayout(const Spec &spec) : m_context{*spec.context}, m_bindings{spec.bindings} {
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings(m_bindings.size());

    size_t index = 0;
    for (const auto &[key, binding] : m_bindings) {
        setLayoutBindings[index] = binding;
        index += 1;
    }

    std::vector<vk::DescriptorBindingFlags> bindingFlags(
        m_bindings.size(), vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind
    );

    vk::DescriptorSetLayoutBindingFlagsCreateInfo setLayoutBindingFlagsCi;
    setLayoutBindingFlagsCi.bindingCount = bindingFlags.size();
    setLayoutBindingFlagsCi.pBindingFlags = bindingFlags.data();

    vk::DescriptorSetLayoutCreateInfo setLayoutCi;
    setLayoutCi.pNext = &setLayoutBindingFlagsCi;
    setLayoutCi.bindingCount = setLayoutBindings.size();
    setLayoutCi.pBindings = setLayoutBindings.data();
    setLayoutCi.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

    auto setLayoutResult = m_context.getDevice().createDescriptorSetLayout(setLayoutCi);
    core::expect(setLayoutResult, "failed to create descriptor set layout");
    m_layout = std::move(*setLayoutResult);

    core::debug("created descriptor set layout");
}

DescriptorSetLayout::~DescriptorSetLayout() { core::debug("destroyed descriptor set layout"); }

auto DescriptorSetLayout::createDescriptorSet(const DescriptorPool &pool) -> std::expected<vk::raii::DescriptorSet, vk::Result> {
    vk::DescriptorSetAllocateInfo descriptorSetAi;
    descriptorSetAi.descriptorPool = pool.get();
    descriptorSetAi.descriptorSetCount = 1;
    descriptorSetAi.pSetLayouts = &(*m_layout);

    auto descriptorSetResult = m_context.getDevice().allocateDescriptorSets(descriptorSetAi);
    if (!descriptorSetResult) {
        return std::unexpected(descriptorSetResult.error());
    }
    return std::move(descriptorSetResult->front());
}

auto DescriptorSetLayout::get() -> vk::raii::DescriptorSetLayout & { return m_layout; }
auto DescriptorSetLayout::get() const -> const vk::raii::DescriptorSetLayout & { return m_layout; }

auto DescriptorSetLayout::getBindings() -> std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> & { return m_bindings; }

} // namespace muon::graphics
