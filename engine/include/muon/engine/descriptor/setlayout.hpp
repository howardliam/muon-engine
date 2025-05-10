#pragma once

#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <memory>

namespace muon::engine {

    class Device;
    class DescriptorPool2;

    class DescriptorSetLayout2 {
    public:
        class Builder;

        DescriptorSetLayout2(
            Device &device,
            const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings
        );
        ~DescriptorSetLayout2();

        vk::DescriptorSet createSet(const DescriptorPool2 &pool);

        vk::DescriptorSetLayout getSetLayout() const;

    private:
        Device &device;

        vk::DescriptorSetLayout setLayout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};

        friend class DescriptorWriter2;
    };

    class DescriptorSetLayout2::Builder {
    public:
        Builder(Device &device);

        Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count = 1);

        DescriptorSetLayout2 build() const;

        std::unique_ptr<DescriptorSetLayout2> buildUniquePtr() const;

    private:
        Device &device;

        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    };
}
