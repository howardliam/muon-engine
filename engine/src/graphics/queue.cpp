#include "muon/graphics/queue.hpp"

#include <string>
#include <vulkan/vulkan_core.h>
#include "muon/core/application.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace {
    std::string ToString(const muon::gfx::QueueType &type) {
        switch (type) {
            case muon::gfx::QueueType::Graphics: {
                return "graphics";
            }

            case muon::gfx::QueueType::Present: {
                return "present";
            }

            case muon::gfx::QueueType::Compute: {
                return "compute";
            }

            case muon::gfx::QueueType::Transfer: {
                return "transfer";
            }
        }
    }
}

namespace muon::gfx {

    std::unordered_set<uint32_t> QueueFamilyIndices::UniqueQueues() const {
        return {graphics, compute, transfer, present};
    }

    std::vector<VkDeviceQueueCreateInfo> QueueFamilyIndices::GenerateQueueCreateInfos() {
        const auto &uniqueQueues = UniqueQueues();

        float queuePriorities[] = {1.0, 1.0};
        uint32_t idx = 0;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueues.size());

        for (const auto &queue : uniqueQueues) {
            queueCreateInfos[idx] = VkDeviceQueueCreateInfo{};
            queueCreateInfos[idx].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfos[idx].pQueuePriorities = queuePriorities;
            queueCreateInfos[idx].queueFamilyIndex = queue;
            if (queue == present && (presentQueueType != QueueType::Present)) {
                queueCreateInfos[idx].queueCount = 2;
            } else {
                queueCreateInfos[idx].queueCount = 1;
            }

            idx += 1;
        }

        return queueCreateInfos;
    }

    QueueFamilyIndices QueueFamilyIndices::DetermineIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyPropertyCount{0};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
        MU_CORE_ASSERT(queueFamilyPropertyCount >= 3, "there must be at least three queue families");

        for (uint32_t i = 0; i < queueFamilyPropertyCount; i++) {
            const auto &properties = queueFamilyProperties[i];

            bool isGraphics = properties.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool isCompute = properties.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool isTransfer = properties.queueFlags & VK_QUEUE_TRANSFER_BIT;

            VkBool32 support;
            auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &support);
            MU_CORE_ASSERT(result == VK_SUCCESS, "failed to get surface support");

            if (isGraphics) {
                indices.graphics = i;
            } else if (isCompute && !isGraphics) {
                indices.compute = i;
            } else if (isTransfer && (!isGraphics && !isCompute)) {
                indices.transfer = i;
            }

            if (support) { indices.present = i; }
        }

        if (indices.present == indices.graphics) {
            indices.presentQueueType = QueueType::Graphics;
        } else if (indices.present == indices.compute) {
            indices.presentQueueType = QueueType::Compute;
        } else if (indices.present == indices.transfer) {
            indices.presentQueueType = QueueType::Transfer;
        } else {
            indices.presentQueueType = QueueType::Present;
        }

        return indices;
    }

    Queue::Queue(QueueType type, VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex) : m_type(type) {
        vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &m_queue);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto result = vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandPool);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create command pool");

        MU_CORE_DEBUG("created {} queue", ToString(m_type));
    }

    Queue::~Queue() {
        vkDestroyCommandPool(Application::Get().GetGraphicsContext().GetDevice(), m_commandPool, nullptr);
        MU_CORE_DEBUG("destroyed {} queue", ToString(m_type));
    }

    VkCommandBuffer Queue::BeginCommands() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        auto result = vkAllocateCommandBuffers(Application::Get().GetGraphicsContext().GetDevice(), &allocateInfo, &commandBuffer);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate single time command buffer");

        VkCommandBufferBeginInfo beginInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to begin command buffer recording");

        return commandBuffer;
    }

    void Queue::EndCommands(VkCommandBuffer cmd) {
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        auto result = vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to submit command buffer to graphics queue");

        vkQueueWaitIdle(m_queue);

        vkFreeCommandBuffers(Application::Get().GetGraphicsContext().GetDevice(), m_commandPool, 1, &cmd);
    }

    QueueType Queue::GetType() const {
        return m_type;
    }

    VkQueue Queue::Get() const {
        return m_queue;
    }

    VkCommandPool Queue::GetCommandPool() const {
        return m_commandPool;
    }

}
