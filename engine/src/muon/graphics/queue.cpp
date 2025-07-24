#include "muon/graphics/queue.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <utility>
#include <vulkan/vulkan_enums.hpp>

namespace muon::graphics {

Queue::Queue(const Spec &spec) : m_device(*spec.device), m_name(spec.name) {
    auto queueResult = m_device.getQueue(spec.queueFamilyIndex, spec.queueIndex);
    core::expect(queueResult, "failed to get queue from device");
    m_queue = std::move(*queueResult);

    vk::CommandPoolCreateInfo commandPoolCi;
    commandPoolCi.queueFamilyIndex = spec.queueFamilyIndex;
    commandPoolCi.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    auto commandPoolResult = m_device.createCommandPool(commandPoolCi);
    core::expect(commandPoolResult, "failed to create command pool for queue");
    m_commandPool = std::move(*commandPoolResult);

    core::debug("created {} queue", m_name);
}

Queue::~Queue() { core::debug("destroyed {} queue", m_name); }

auto Queue::ExecuteCommands(std::function<void(vk::raii::CommandBuffer &commandBuffer)> const &recordFn) {
    vk::CommandBufferAllocateInfo commandBufferAi;
    commandBufferAi.level = vk::CommandBufferLevel::ePrimary;
    commandBufferAi.commandPool = m_commandPool;
    commandBufferAi.commandBufferCount = 1;

    auto commandBufferResult = m_device.allocateCommandBuffers(commandBufferAi);
    core::expect(commandBufferResult, "failed to allocate single time command buffer");
    auto &commandBuffer = (*commandBufferResult)[0];

    vk::CommandBufferBeginInfo commandBufferBi;
    commandBufferBi.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    commandBuffer.begin(commandBufferBi);

    recordFn(commandBuffer);

    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(*commandBuffer);

    m_queue.submit(submitInfo);

    m_queue.waitIdle();
}

auto Queue::GetFamilyIndex() const -> uint32_t { return m_familyIndex; }

auto Queue::GetIndex() const -> uint32_t { return m_index; }

auto Queue::Get() -> vk::raii::Queue & { return m_queue; }
auto Queue::Get() const -> const vk::raii::Queue & { return m_queue; }

auto Queue::GetCommandPool() -> vk::raii::CommandPool & { return m_commandPool; }
auto Queue::GetCommandPool() const -> const vk::raii::CommandPool & { return m_commandPool; }

} // namespace muon::graphics
