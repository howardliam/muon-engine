#pragma once

#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <memory>

namespace mu {

    class Device;
    class DescriptorPool;

    /**
     * @brief   Wrapper around Vulkan descriptor set layout.
     */
    class DescriptorSetLayout {
    public:
        class Builder;

        DescriptorSetLayout(
            Device &device,
            const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings
        );
        ~DescriptorSetLayout();

        /**
         * @brief   Allocate new descriptor set from layout.
         *
         * @param   pool    reference to DescriptorPool for allocating the descriptor set.
         *
         * @return  descriptor set handle.
         */
        [[nodiscard]] vk::DescriptorSet createSet(const DescriptorPool &pool);

        /**
         * @brief   Gets the descriptor set layout handle.
         *
         * @return  descriptor set layout handle.
         */
        [[nodiscard]] vk::DescriptorSetLayout getSetLayout() const;

    private:
        Device &device;

        vk::DescriptorSetLayout setLayout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};

        friend class DescriptorWriter;
    };

    class DescriptorSetLayout::Builder {
    public:
        Builder(Device &device);

        /**
         * @brief   Adds a descriptor binding in the descriptor set layout.
         *
         * @param   binding         binding location for the descriptor.
         * @param   descriptorType  the type of descriptor to set binding to.
         * @param   stageFlags      which pipeline stages can access the descriptor.
         * @param   count           how many there will be (>1 is an array).
         *
         * @return  reference to the builder.
         */
        Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count = 1);

        /**
         * @brief   Builds a new descriptor set layout from the configuration.
         *
         * @return  new DescriptorSetLayout object.
         */
        [[nodiscard]] DescriptorSetLayout build() const;

        /**
         * @brief   Builds a new descriptor set layout from the configuration.
         *
         * @return  unique pointer to the new DescriptorSetLayout object.
         */
        [[nodiscard]] std::unique_ptr<DescriptorSetLayout> buildUniquePtr() const;

    private:
        Device &device;

        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    };
}
