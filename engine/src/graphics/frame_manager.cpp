#include "muon/graphics/frame_manager.hpp"

#include "muon/core/application.hpp"
#include <vulkan/vulkan_core.h>

namespace muon::gfx {

    FrameManager::FrameManager() {
        CreateSwapchain();
        CreateCommandBuffers();
    }

    FrameManager::~FrameManager() {
        auto &context = Application::Get().GetGraphicsContext();
        vkFreeCommandBuffers(
            context.GetDevice(),
            context.GetGraphicsQueue().GetCommandPool(),
            m_commandBuffers.size(),
            m_commandBuffers.data()
        );
        m_commandBuffers.clear();
    }

    VkCommandBuffer FrameManager::BeginFrame() {
        MU_CORE_ASSERT(!m_frameInProgress, "cannot begin frame while frame is in progress");

        auto result = m_swapchain->AcquireNextImage(&m_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            CreateSwapchain();
            return nullptr;
        }
        MU_CORE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "failed to acquire next swapchain image");

        m_frameInProgress = true;

        const auto cmd = m_commandBuffers[m_currentFrameIndex];

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        result = vkBeginCommandBuffer(cmd, &beginInfo);
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to begin recording command buffer");

        return cmd;
    }

    void FrameManager::EndFrame() {
        MU_CORE_ASSERT(m_frameInProgress, "cannot end frame if a frame has not been started");

        const auto cmd = m_commandBuffers[m_currentFrameIndex];
        vkEndCommandBuffer(cmd);

        auto result = m_swapchain->SubmitCommandBuffers(&cmd, &m_currentImageIndex);
        MU_CORE_ASSERT(
            result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR ||
            result == VK_SUCCESS,
            "failed to present swapchain image"
        );
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            CreateSwapchain();
        }

        m_frameInProgress = false;
        m_currentFrameIndex = (m_currentFrameIndex + 1) % constants::maxFramesInFlight;
    }

    void FrameManager::CreateSwapchain() {
        auto extent = Application::Get().GetWindow().GetExtent();

        if (m_swapchain == nullptr) {
            m_swapchain = std::make_unique<Swapchain>(extent);
        } else {
            std::shared_ptr oldSwapChain = std::move(m_swapchain);
            m_swapchain = std::make_unique<Swapchain>(extent, oldSwapChain);

            if (!m_swapchain->CompareSwapFormats(*oldSwapChain)) {
                MU_CORE_DEBUG("new and old swapchain formats do not match");
            }
        }
    }

    void FrameManager::CreateCommandBuffers() {
        auto &context = Application::Get().GetGraphicsContext();

        m_commandBuffers.resize(constants::maxFramesInFlight);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level =  VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = context.GetGraphicsQueue().GetCommandPool();
        allocInfo.commandBufferCount = m_commandBuffers.size();

        auto result = vkAllocateCommandBuffers(context.GetDevice(), &allocInfo, m_commandBuffers.data());
        MU_CORE_ASSERT(result == VK_SUCCESS, "failed to allocate command buffers");
    }

}
