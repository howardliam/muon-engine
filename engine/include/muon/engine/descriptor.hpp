#pragma once

#include "muon/engine/device.hpp"
#include <memory>
#include <unordered_map>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    /**
     * @brief wrapper around vulkan descriptor pool for managing the creation of descriptors.
     */
    class DescriptorPool {
    public:
        class Builder {
        public:
            explicit Builder(Device &device);

            /**
             * @brief   sets how many of a descriptor type can be allocated from the pool.
             *
             * @param   descriptorType  the type of descriptor to set a pool size for.
             * @param   size            the size of the pool.
             *
             * @return  reference to Builder.
             */
            Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t size);

            /**
             * @brief   sets create flags for the pool.
             *
             * @param   flags   the flags to use.
             *
             * @return  reference to Builder.
             */
            Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);

            /**
             * @brief   sets how many of a descriptor sets can be allocated from the pool.
             *
             * @param   count   max amount of descriptor sets to be allocated.
             *
             * @return  reference to Builder.
             */
            Builder &setMaxSets(uint32_t count);

            /**
             * @brief   builds the descriptor pool from the provided information.
             *
             * @return  unique pointer to the DescriptorPool object.
             */
            std::unique_ptr<DescriptorPool> build() const;

        private:
            Device &device;

            std::vector<vk::DescriptorPoolSize> poolSizes{};
            uint32_t maxSets{1000};
            vk::DescriptorPoolCreateFlags poolFlags{};
        };

        DescriptorPool(Device &device, uint32_t maxSets, vk::DescriptorPoolCreateFlags poolFlags, const std::vector<vk::DescriptorPoolSize> &poolSizes);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        /**
         * @brief   allocates the descriptor set.
         *
         * @param   descriptorSetLayout  the layout of the descriptor set.
         * @param   descriptorSet        the descriptor set handle to allocate to.
         *
         * @return  whether the allcation was successful.
         */
        bool allocateDescriptorSet(const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet &descriptorSet) const;

        /**
         * @brief   frees the vector of descriptor set handles.
         *
         * @param   descriptors vector of descriptor sets to be freed.
         */
        void freeDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSets) const;

        /**
         * @brief   resets the descriptor pool; frees all descriptor sets allocated from this pool.
         */
        void resetPool();

        /**
         * @brief   gets the descriptor pool handle.
         *
         * @return  descriptor pool handle.
         */
        vk::DescriptorPool getDescriptorPool() const;

    private:
        Device &device;

        vk::DescriptorPool descriptorPool;

        friend class DescriptorWriter;
    };

    /**
     * @brief wrapper around vulkan descriptor set layout for managing the creation of descriptor set layouts.
     */
    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            explicit Builder(Device &device);

            /**
             * @brief   adds a binding.
             *
             * @param   binding         the binding location in the shader.
             * @param   descriptorType  the type of descriptor: image sampler, buffer, etc.
             * @param   stageFlags      which pipeline stages is the descriptor set required for.
             * @param   count           how many descriptors in the binding (accessed like an array in the shader).
             *
             * @return  reference to Builder.
             */
            Builder &addBinding(uint32_t binding, vk::DescriptorType descriptorType, vk::ShaderStageFlags stageFlags, uint32_t count = 1);

            Builder &setFlags(vk::DescriptorSetLayoutCreateFlags flags);

            Builder &setBindless(bool bindless);

            /**
             * @brief   builds the descriptor set layout from the provided information.
             *
             * @return  unique pointer to the DescriptorSetLayout object.
             */
            std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            Device &device;

            std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
            vk::DescriptorSetLayoutCreateFlags flags{};
            bool bindless{false};
        };

        DescriptorSetLayout(
            Device &device,
            const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> &bindings,
            vk::DescriptorSetLayoutCreateFlags flags,
            bool bindless
        );
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        /**
         * @brief   gets the descriptor set layout handle.
         *
         * @return  descriptor set layout handle.
         */
        vk::DescriptorSetLayout getDescriptorSetLayout() const;

    private:
        Device &device;

        vk::DescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};

        friend class DescriptorWriter;
    };

    /**
     * @brief   helper to write data to descriptors.
     */
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

        /**
         * @brief   writes buffer info into descriptor at binding location in descriptor set.
         *
         * @param   binding     binding location in the shader.
         * @param   bufferInfo  buffer info for descriptor.
         *
         * @return  reference to DescriptorWriter.
         */
        DescriptorWriter &writeBuffer(uint32_t binding, vk::DescriptorBufferInfo *bufferInfo);

        /**
         * @brief   writes image info into descriptor at binding location in descriptor set.
         *
         * @param   binding     binding location in the shader.
         * @param   imageInfo   image info for descriptor.
         *
         * @return  reference to DescriptorWriter.
         */
        DescriptorWriter &writeImage(uint32_t binding, vk::DescriptorImageInfo *imageInfo);

        /**
         * @brief   builds the descriptor set based on the info written to it.
         *
         * @param   set handle for the descriptor set to be created at.
         *
         * @return  whether it was successful.
         */
        bool build(vk::DescriptorSet &set);

        /**
         * @brief   updates and overwrites data in descriptor set.
         *
         * @param   set handle for the descriptor to have its data updated.
         */
        void overwrite(vk::DescriptorSet &set);

    private:
        DescriptorSetLayout &setLayout;
        DescriptorPool &pool;
        std::vector<vk::WriteDescriptorSet> writes;
    };

}
