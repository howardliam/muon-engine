#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <memory>

namespace muon::engine {

    class Device;

    class DescriptorPool {
    public:
        class Builder;

        DescriptorPool(
            Device &device,
            uint32_t maxSets,
            const std::vector<vk::DescriptorPoolSize> &poolSizes
        );
        ~DescriptorPool();

        vk::DescriptorPool getPool() const;
    private:
        Device &device;

        vk::DescriptorPool pool;

        friend class DescriptorWriter;
    };

    class DescriptorPool::Builder {
    public:
        Builder(Device &device);

        Builder &addPoolSize(vk::DescriptorType descriptorType, uint32_t size);

        Builder &setMaxSets(uint32_t count);

        DescriptorPool build() const;

        std::unique_ptr<DescriptorPool> buildUniquePtr() const;

    private:
        Device &device;

        std::vector<vk::DescriptorPoolSize> poolSizes{};
        uint32_t maxSets{1000};
    };

}
