#include "muon/engine/renderer/descriptor/pool.hpp"

#include "muon/engine/core/assert.hpp"
#include "muon/engine/core/log.hpp"
#include "muon/engine/renderer/device.hpp"

namespace muon {

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

        auto result = device.device().createDescriptorPool(&createInfo, nullptr, &pool);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create descriptor pool");

        MU_CORE_DEBUG("created descriptor pool");
    }

    DescriptorPool::~DescriptorPool() {
        device.device().destroyDescriptorPool(pool);
        MU_CORE_DEBUG("destroyed descriptor pool");
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
