#include "muon/engine/renderer/descriptor/pool.hpp"

#include "muon/engine/core/assert.hpp"
#include "muon/engine/core/log.hpp"
#include "muon/engine/renderer/device.hpp"

namespace muon {

    DescriptorPool::DescriptorPool(
        Device &device,
        uint32_t maxSets,
        const std::vector<vk::DescriptorPoolSize> &poolSizes
    ) : m_device(device) {
        vk::DescriptorPoolCreateInfo createInfo{};
        createInfo.maxSets = maxSets;
        createInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        createInfo.pPoolSizes = poolSizes.data();

        auto result = m_device.device().createDescriptorPool(&createInfo, nullptr, &m_pool);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create descriptor pool");

        MU_CORE_DEBUG("created descriptor pool");
    }

    DescriptorPool::~DescriptorPool() {
        m_device.device().destroyDescriptorPool(m_pool);
        MU_CORE_DEBUG("destroyed descriptor pool");
    }

    vk::DescriptorPool DescriptorPool::pool() const {
        return m_pool;
    }

    DescriptorPool::Builder::Builder(Device &device) : m_device(device) {}

    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t size) {
        m_poolSizes.push_back(vk::DescriptorPoolSize{ descriptorType, size });
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
        m_maxSets = count;
        return *this;
    }

    DescriptorPool DescriptorPool::Builder::build() const {
        return DescriptorPool(m_device, m_maxSets, m_poolSizes);
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::buildUniquePtr() const {
        return std::make_unique<DescriptorPool>(m_device, m_maxSets, m_poolSizes);
    }

}
