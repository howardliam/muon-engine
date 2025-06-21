#include "muon/graphics/queue.hpp"

#include <vulkan/vulkan_core.h>
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

namespace muon::gfx {

    Queue::Queue(const QueueSpecification &spec) : m_device(spec.device), m_name(spec.name) {
        vkGetDeviceQueue(m_device, spec.queueFamilyIndex, spec.queueIndex, &m_queue);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = spec.queueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to create command pool");

        MU_CORE_DEBUG("created {} queue", m_name);
    }

    Queue::~Queue() {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        MU_CORE_DEBUG("destroyed {} queue", m_name);
    }

    VkCommandBuffer Queue::BeginCommands() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        auto result = vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate single time command buffer");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
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

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &cmd);
    }

    VkQueue Queue::Get() const {
        return m_queue;
    }

    VkCommandPool Queue::GetCommandPool() const {
        return m_commandPool;
    }

}
