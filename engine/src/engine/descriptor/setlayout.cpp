#include "muon/engine/descriptor.hpp"

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

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings);
    }

    DescriptorSetLayout::DescriptorSetLayout(Device &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings) : device(device), bindings(bindings) {
        std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto [key, value] : bindings) {
            setLayoutBindings.push_back(value);
        }

        vk::DescriptorSetLayoutCreateInfo createInfo{};
        createInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        createInfo.pBindings = setLayoutBindings.data();

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
