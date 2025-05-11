#include "muon/engine/descriptor.hpp"
#include "muon/engine/descriptor/pool.hpp"
#include "muon/engine/device.hpp"
#include "muon/log/logger.hpp"

#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    DescriptorPool::Builder::Builder(Device &device) : device(device) {}

    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t size) {
        poolSizes.push_back({ descriptorType, size });
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    DescriptorPool::DescriptorPool(
        Device &device,
        uint32_t maxSets,
        vk::DescriptorPoolCreateFlags poolFlags,
        const std::vector<vk::DescriptorPoolSize> &poolSizes
    ) : device(device) {
        vk::DescriptorPoolCreateInfo createInfo{};
        createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        createInfo.pPoolSizes = poolSizes.data();
        createInfo.maxSets = maxSets;
        createInfo.flags = poolFlags;

        auto result = device.getDevice().createDescriptorPool(&createInfo, nullptr, &descriptorPool);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create descriptor pool");
        }
    }

    DescriptorPool::~DescriptorPool() {
        device.getDevice().destroyDescriptorPool(descriptorPool, nullptr);
    }

    bool DescriptorPool::allocateDescriptorSet(const vk::DescriptorSetLayout descriptorSetLayout, vk::DescriptorSet &descriptorSet) const {
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        auto result = device.getDevice().allocateDescriptorSets(&allocInfo, &descriptorSet);
        if (result != vk::Result::eSuccess) {
            return false;
        }
        return true;
    }

    void DescriptorPool::freeDescriptorSets(std::vector<vk::DescriptorSet> &descriptorSets) const {
        device.getDevice().freeDescriptorSets(descriptorPool, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
    }

    void DescriptorPool::resetPool() {
        device.getDevice().resetDescriptorPool(descriptorPool, vk::DescriptorPoolResetFlags{});
    }

    vk::DescriptorPool DescriptorPool::getDescriptorPool() const {
        return descriptorPool;
    }

    /* NEW API */

    DescriptorPool2::DescriptorPool2(
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

    DescriptorPool2::~DescriptorPool2() {
        device.getDevice().destroyDescriptorPool(pool);
        log::globalLogger->debug("destroyed descriptor pool");
    }

    vk::DescriptorPool DescriptorPool2::getPool() const {
        return pool;
    }

    DescriptorPool2::Builder::Builder(Device &device) : device(device) {}

    DescriptorPool2::Builder &DescriptorPool2::Builder::addPoolSize(vk::DescriptorType descriptorType, uint32_t size) {
        poolSizes.push_back(vk::DescriptorPoolSize{ descriptorType, size });
        return *this;
    }

    DescriptorPool2::Builder &DescriptorPool2::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    DescriptorPool2 DescriptorPool2::Builder::build() const {
        return DescriptorPool2(device, maxSets, poolSizes);
    }

    std::unique_ptr<DescriptorPool2> DescriptorPool2::Builder::buildUniquePtr() const {
        return std::make_unique<DescriptorPool2>(device, maxSets, poolSizes);
    }

}
