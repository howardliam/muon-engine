#pragma once

#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <memory>

namespace muon::engine {

    class Device;
    class DescriptorPool;

    class DescriptorSetLayout {
    public:
        class Builder;

        DescriptorSetLayout(
            Device &device,
            const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings
        );
        ~DescriptorSetLayout();

        vk::DescriptorSet createSet(const DescriptorPool &pool);

        vk::DescriptorSetLayout getSetLayout() const;

    private:
        Device &device;

        vk::DescriptorSetLayout setLayout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};

        friend class DescriptorWriter;
    };

    class DescriptorSetLayout::Builder {
    public:
        Builder(Device &device);

        Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count = 1);

        DescriptorSetLayout build() const;

        std::unique_ptr<DescriptorSetLayout> buildUniquePtr() const;

    private:
        Device &device;

        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    };
}
