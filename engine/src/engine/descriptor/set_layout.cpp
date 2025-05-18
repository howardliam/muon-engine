#include "muon/engine/descriptor/set_layout.hpp"

#include "muon/engine/descriptor/pool.hpp"
#include "muon/engine/device.hpp"

namespace muon::engine {

    DescriptorSetLayout::DescriptorSetLayout(
        Device &device,
        const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings
    ) : device(device), bindings(bindings) {
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

        auto result = device.getDevice().createDescriptorSetLayout(&createInfo, nullptr, &setLayout);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create descriptor set layout");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        device.getDevice().destroyDescriptorSetLayout(setLayout, nullptr);
    }

    vk::DescriptorSet DescriptorSetLayout::createSet(const DescriptorPool &pool) {
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.descriptorPool = pool.getPool();
        allocInfo.pSetLayouts = &setLayout;
        allocInfo.descriptorSetCount = 1;

        vk::DescriptorSet set;
        auto _ = device.getDevice().allocateDescriptorSets(&allocInfo, &set);
        return set;
    }

    vk::DescriptorSetLayout DescriptorSetLayout::getSetLayout() const {
        return setLayout;
    }

    DescriptorSetLayout::Builder::Builder(Device &device) : device(device) {}

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

        bindings[binding] = layoutBinding;

        return *this;
    }

    DescriptorSetLayout DescriptorSetLayout::Builder::build() const {
        return DescriptorSetLayout(device, bindings);
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::buildUniquePtr() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings);
    }

}
