#include "muon/renderer/descriptor/set_layout.hpp"

#include "muon/core/assert.hpp"
#include "muon/renderer/descriptor/pool.hpp"
#include "muon/renderer/device.hpp"

namespace muon {

    DescriptorSetLayout::DescriptorSetLayout(
        Device &device,
        const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings
    ) : m_device(device), m_bindings(bindings) {
        std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings(bindings.size());

        size_t index = 0;
        for (const auto &[key, binding] : bindings) {
            setLayoutBindings[index] = binding;
            index += 1;
        }

        std::vector<vk::DescriptorBindingFlags> bindingFlags(
            bindings.size(),
            // vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
            vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind
        );

        vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingInfo{};
        bindingInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
        bindingInfo.pBindingFlags = bindingFlags.data();

        vk::DescriptorSetLayoutCreateInfo createInfo{};
        createInfo.pNext = &bindingInfo;
        createInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        createInfo.pBindings = setLayoutBindings.data();
        createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

        auto result = m_device.device().createDescriptorSetLayout(&createInfo, nullptr, &m_setLayout);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create descriptor set layout");
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        m_device.device().destroyDescriptorSetLayout(m_setLayout, nullptr);
    }

    vk::DescriptorSet DescriptorSetLayout::createSet(const DescriptorPool &pool) {
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.descriptorPool = pool.pool();
        allocInfo.pSetLayouts = &m_setLayout;
        allocInfo.descriptorSetCount = 1;

        vk::DescriptorSet set;
        auto _ = m_device.device().allocateDescriptorSets(&allocInfo, &set);
        return set;
    }

    vk::DescriptorSetLayout DescriptorSetLayout::setLayout() const {
        return m_setLayout;
    }

    DescriptorSetLayout::Builder::Builder(Device &device) : m_device(device) {}

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        vk::DescriptorType descriptorType,
        vk::ShaderStageFlags stageFlags,
        uint32_t count
    ) {
        vk::DescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;

        m_bindings[binding] = layoutBinding;

        return *this;
    }

    DescriptorSetLayout DescriptorSetLayout::Builder::build() const {
        return DescriptorSetLayout(m_device, m_bindings);
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::buildUniquePtr() const {
        return std::make_unique<DescriptorSetLayout>(m_device, m_bindings);
    }

}
