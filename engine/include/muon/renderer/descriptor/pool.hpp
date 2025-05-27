#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <memory>

namespace muon {

    class Device;

    /**
     * @brief   Wrapper around Vulkan descriptor pool.
     */
    class DescriptorPool {
    public:
        class Builder;

        DescriptorPool(
            Device &device,
            uint32_t maxSets,
            const std::vector<vk::DescriptorPoolSize> &poolSizes
        );
        ~DescriptorPool();

        /**
         * @brief   Gets the descriptor pool handle.
         *
         * @return  descriptor pool handle.
         */
        [[nodiscard]] vk::DescriptorPool pool() const;

    private:
        Device &m_device;

        vk::DescriptorPool m_pool;

        friend class DescriptorWriter;
    };

    class DescriptorPool::Builder {
    public:
        Builder(Device &device);

        /**
         * @brief   Adds a pool size for a descriptor type.
         *
         * @param   descriptorType  the type of descriptor to set the pool size for.
         * @param   size            how large the pool will be.
         *
         * @return  reference to the builder.
         */
        Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t size);

        /**
         * @brief   Sets the maximum number of descriptor sets which can be allocated from the pool.
         *
         * @param   count   max number of descriptor sets allowed to be allocated.
         *
         * @return  reference to the builder.
         */
        Builder &setMaxSets(uint32_t count);

        /**
         * @brief   Builds a new descriptor pool from the configuration.
         *
         * @return  new DescriptorPool object.
         */
        [[nodiscard]] DescriptorPool build() const;

        /**
         * @brief   Builds a new descriptor pool from the configuration.
         *
         * @return  unique pointer to the new DescriptorPool object.
         */
        [[nodiscard]] std::unique_ptr<DescriptorPool> buildUniquePtr() const;

    private:
        Device &m_device;

        std::vector<vk::DescriptorPoolSize> m_poolSizes{};
        uint32_t m_maxSets{1000};
    };

}
