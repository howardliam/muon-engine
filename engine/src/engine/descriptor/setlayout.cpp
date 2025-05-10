#include "muon/engine/descriptor.hpp"
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

}
