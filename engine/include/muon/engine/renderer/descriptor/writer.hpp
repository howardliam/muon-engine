#pragma once

#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace muon {

    class DescriptorPool;
    class DescriptorSetLayout;

    /**
     * @brief   Helper class for writing to descriptor sets.
     */
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorPool &pool, DescriptorSetLayout &setLayout);

        /**
         * @brief   Adds a new buffer write to the vector of writes.
         *
         * @param   binding     binding location in the descriptor set layout.
         * @param   position    position in descriptor array.
         * @param   bufferInfo  pointer to buffer info.
         *
         * @return  reference to the writer.
         */
        DescriptorWriter &addBufferWrite(uint32_t binding, size_t position, vk::DescriptorBufferInfo *bufferInfo);

        /**
         * @brief   Adds a new image write to the vector of writes.
         *
         * @param   binding     binding location in the descriptor set layout.
         * @param   position    position in descriptor array.
         * @param   imageInfo   pointer to image info.
         *
         * @return  reference to the writer.
         */
        DescriptorWriter &addImageWrite(uint32_t binding, size_t position, vk::DescriptorImageInfo *imageInfo);

        /**
         * @brief   Writes all the queued writes to the provided descriptor set.
         *
         * @param   set handle to the descriptor set to write to.
         */
        void writeAll(vk::DescriptorSet set);

    private:
        DescriptorPool &pool;
        DescriptorSetLayout &setLayout;
        std::vector<vk::WriteDescriptorSet> writes;
    };

}
