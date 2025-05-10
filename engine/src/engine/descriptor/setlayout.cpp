#include "muon/engine/descriptor.hpp"
#include "muon/engine/descriptor/setlayout.hpp"

#include "muon/engine/descriptor/pool.hpp"
#include "muon/engine/device.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace muon::engine {

    DescriptorSetLayout::Builder::Builder(Device &device) : device(device) {}

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count) {
        vk::DescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;

        bindings[binding] = layoutBinding;

        return *this;
    }

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::setFlags(vk::DescriptorSetLayoutCreateFlags flags) {
        this->flags = flags;
        return *this;
    }

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::setBindless(bool bindless) {
        this->bindless = bindless;
        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings, flags, bindless);
    }

    DescriptorSetLayout::DescriptorSetLayout(
        Device &device,
        const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings,
        vk::DescriptorSetLayoutCreateFlags flags,
        bool bindless
    ) : device(device), bindings(bindings) {
        std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings(bindings.size());

        size_t index = 0;
        for (const auto &[key, binding] : bindings) {
            setLayoutBindings[index] = binding;
            index += 1;
        }

        vk::DescriptorSetLayoutCreateInfo createInfo{};
        createInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        createInfo.pBindings = setLayoutBindings.data();
        createInfo.flags = flags;

        vk::DescriptorBindingFlags bindingFlags =
            vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
            vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind;

        vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT bindingInfo{};
        bindingInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        bindingInfo.pBindingFlags = &bindingFlags;

        if (bindless) {
            createInfo.pNext = &bindingInfo;
        }

        auto result = device.getDevice().createDescriptorSetLayout(&createInfo, nullptr, &descriptorSetLayout);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create descriptor set layout");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        device.getDevice().destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
    }

    vk::DescriptorSetLayout DescriptorSetLayout::getDescriptorSetLayout() const {
        return descriptorSetLayout;
    }

    /* NEW API */

    DescriptorSetLayout2::DescriptorSetLayout2(
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

    DescriptorSetLayout2::~DescriptorSetLayout2() {
        device.getDevice().destroyDescriptorSetLayout(setLayout, nullptr);
    }

    vk::DescriptorSet DescriptorSetLayout2::createSet(const DescriptorPool2 &pool) {
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.descriptorPool = pool.getPool();
        allocInfo.pSetLayouts = &setLayout;
        allocInfo.descriptorSetCount = 1;

        vk::DescriptorSet set;
        auto _ = device.getDevice().allocateDescriptorSets(&allocInfo, &set);
        return set;
    }

    vk::DescriptorSetLayout DescriptorSetLayout2::getSetLayout() const {
        return setLayout;
    }

    DescriptorSetLayout2::Builder::Builder(Device &device) : device(device) {}

    DescriptorSetLayout2::Builder &DescriptorSetLayout2::Builder::addBinding(
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

    DescriptorSetLayout2 DescriptorSetLayout2::Builder::build() const {
        return DescriptorSetLayout2(device, bindings);
    }

    std::unique_ptr<DescriptorSetLayout2> DescriptorSetLayout2::Builder::buildUniquePtr() const {
        return std::make_unique<DescriptorSetLayout2>(device, bindings);
    }

}
