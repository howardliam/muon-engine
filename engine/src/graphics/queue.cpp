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
