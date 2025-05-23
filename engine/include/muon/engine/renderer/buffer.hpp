#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

namespace mu {

    class Device;

    /**
     * @brief wrapper around vulkan buffer.
     */
    class Buffer : NoCopy, NoMove {
    public:
        Buffer(
            Device &device,
            vk::DeviceSize instanceSize,
            uint32_t instanceCount,
            vk::BufferUsageFlags usageFlags,
            vma::MemoryUsage memoryUsage,
            vk::DeviceSize minOffsetAlignment = 1
        );
        ~Buffer();

        /**
         * @brief   maps the buffer memory.
         *
         * @return  the result of the mapping.
         */
        [[nodiscard]] vk::Result map();

        /**
         * @brief   unmaps the buffer memory.
         */
        void unmap();

        /**
         * @brief   writes data into buffer, optionally with size and offset.
         *
         * @param   data    the data to write into the buffer.
         * @param   size    optional size of memory to write, defaults to all.
         * @param   offset  optional offset to write into the buffer, defaults to beginning.
         */
        void writeToBuffer(void *data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief   writes data at a particular index into the buffer.
         *
         * @param   data    the data to write.
         * @param   index
         */
        void writeToIndex(void *data, int32_t index);

        /**
         * @brief   flushes data from buffer to GPU memory.
         *
         * @param   size    optional size of memory to flush, defaults to all.
         * @param   offset  optional offset to flush into the GPU memory, defaults to beginning.
         */
        void flush(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief   flushes data from buffer to GPU memory at index.
         *
         * @param   index   the index to flush from.
         */
        void flushIndex(int32_t index);

        /**
         * @brief   creates information for descriptors to use about the buffer.
         *
         * @param   size    optional size of memory to use, defaults to all.
         * @param   offset  optional offset, defaults to beginning.
         *
         * @return  struct for use with descriptors.
         */
        [[nodiscard]] vk::DescriptorBufferInfo getDescriptorInfo(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief   creates information for descriptors at index.
         *
         * @param   index   the index to create the information from.
         *
         * @return  struct for use with descriptors.
         */
        [[nodiscard]] vk::DescriptorBufferInfo getDescriptorInfoForIndex(int32_t index);

        /**
         * @brief   invalidates the allocated buffer.
         *
         * @param   size    optional size of memory to use, defaults to all.
         * @param   offset  optional offset, defaults to beginning.
         */
        void invalidate(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);

        /**
         * @brief   invalidates the allocated buffer from the index.
         *
         * @param   index   the index to invalidate from.
         */
        void invalidateIndex(int32_t index);

        /**
         * @brief   gets the buffer handle.
         *
         * @return  buffer handle.
         */
        [[nodiscard]] vk::Buffer getBuffer() const;

        /**
         * @brief
         */
        [[nodiscard]] vk::DeviceSize getBufferSize() const;

        /**
         * @brief
         */
        void *getMappedMemory() const;

        /**
         * @brief
         */
        [[nodiscard]] uint32_t getInstanceCount() const;

        /**
         * @brief
         */
        [[nodiscard]] vk::DeviceSize getInstanceSize() const;

        /**
         * @brief
         */
        [[nodiscard]] vk::DeviceSize getAlignmentSize() const;

        /**
         * @brief
         */
        [[nodiscard]] vk::BufferUsageFlags getUsageFlags() const;


    private:
        Device &device;

        vk::Buffer buffer;
        vk::DeviceSize bufferSize;
        vma::Allocation allocation;
        void *mapped{nullptr};

        uint32_t instanceCount;
        vk::DeviceSize instanceSize;
        vk::DeviceSize alignmentSize;
        vk::BufferUsageFlags usageFlags;
    };

}
