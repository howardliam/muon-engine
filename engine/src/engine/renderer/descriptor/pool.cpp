#include "muon/engine/renderer/descriptor/pool.hpp"

#include "muon/engine/renderer/device.hpp"
#include "muon/engine/log/logger.hpp"

#include <stdexcept>

namespace muon::engine {

    DescriptorPool::DescriptorPool(
        Device &device,
        uint32_t maxSets,
        const std::vector<vk::DescriptorPoolSize> &poolSizes
    ) : device(device) {
        vk::DescriptorPoolCreateInfo createInfo{};
        createInfo.maxSets = maxSets;
        createInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        createInfo.pPoolSizes = poolSizes.data();

        auto result = device.getDevice().createDescriptorPool(&createInfo, nullptr, &pool);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create descriptor pool");
        }

        log::globalLogger->debug("created descriptor pool");
    }

    DescriptorPool::~DescriptorPool() {
        device.getDevice().destroyDescriptorPool(pool);
        log::globalLogger->debug("destroyed descriptor pool");
    }

    vk::DescriptorPool DescriptorPool::getPool() const {
        return pool;
    }

    DescriptorPool::Builder::Builder(Device &device) : device(device) {}

    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t size) {
        poolSizes.push_back(vk::DescriptorPoolSize{ descriptorType, size });
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    DescriptorPool DescriptorPool::Builder::build() const {
        return DescriptorPool(device, maxSets, poolSizes);
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::buildUniquePtr() const {
        return std::make_unique<DescriptorPool>(device, maxSets, poolSizes);
    }

}
