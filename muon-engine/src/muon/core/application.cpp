#include "muon/core/application.hpp"

#include "muon/asset/loaders/png.hpp"
#include "muon/core/debug.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "spdlog/common.h"
#include "vulkan/vulkan_enums.hpp"

#include <memory>

namespace muon {

Application::Application(
    const std::string_view name,
    const vk::Extent2D &extent,
    const bool vsync,
    const WindowMode mode
) : m_name{name} {
    core::expect(!s_instance, "application already exists");
    s_instance = this;

    auto logLevel = k_debugEnabled ? spdlog::level::trace : spdlog::level::info;
    Log::setLogLevel(logLevel);

    m_dispatcher = std::make_unique<event::Dispatcher>();

    m_window = std::make_unique<Window>(m_name, extent, mode, *m_dispatcher);

    graphics::Context::Spec contextSpec{*m_window};
    contextSpec.debug = k_debugEnabled;
    m_context = std::make_unique<graphics::Context>(contextSpec);

    graphics::Renderer::Spec rendererSpec{*m_window, *m_context};
    rendererSpec.vsync = vsync;
    m_renderer = std::make_unique<graphics::Renderer>(rendererSpec);

    asset::AssetManager::Spec assetManagerSpec{*m_context};
    assetManagerSpec.loaders = {new asset::PngLoader()};
    m_assetManager = std::make_unique<asset::AssetManager>(assetManagerSpec);

    m_onWindowClose = m_dispatcher->subscribe<event::WindowCloseEvent>([&](const auto &event) { m_running = false; });

    core::debug("created application");
}

Application::~Application() { core::debug("destroyed application"); }

auto Application::pushLayer(Layer *layer) -> void {
    m_layerStack.pushLayer(layer);
    layer->onAttach();
}

auto Application::run() -> void {
    core::expect(!m_running, "application cannot already be running");
    m_running = true;
    core::info("running application");

    while (m_running) {
        m_window->pollEvents();

        for (auto &layer : m_layerStack) {
            layer->onUpdate();
        }

        if (auto cmd = m_renderer->beginFrame(); cmd) {
            auto &commandBuffer = **cmd;

            vk::ImageMemoryBarrier2 resetImb;
            resetImb.image = m_renderer->getCurrentSwapchainImage();
            resetImb.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            resetImb.subresourceRange.baseMipLevel = 0;
            resetImb.subresourceRange.levelCount = 1;
            resetImb.subresourceRange.baseArrayLayer = 0;
            resetImb.subresourceRange.layerCount = 1;

            resetImb.oldLayout = vk::ImageLayout::eUndefined;
            resetImb.srcAccessMask = vk::AccessFlagBits2::eNone;
            resetImb.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            resetImb.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;

            resetImb.newLayout = vk::ImageLayout::eTransferDstOptimal;
            resetImb.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
            resetImb.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            resetImb.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;

            vk::DependencyInfo resetDi;
            resetDi.dependencyFlags = vk::DependencyFlags{};
            resetDi.imageMemoryBarrierCount = 1;
            resetDi.pImageMemoryBarriers = &resetImb;
            commandBuffer.pipelineBarrier2(resetDi);

            vk::ImageMemoryBarrier2 presentImb;
            presentImb.image = m_renderer->getCurrentSwapchainImage();
            presentImb.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            presentImb.subresourceRange.baseMipLevel = 0;
            presentImb.subresourceRange.levelCount = 1;
            presentImb.subresourceRange.baseArrayLayer = 0;
            presentImb.subresourceRange.layerCount = 1;

            presentImb.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            presentImb.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
            presentImb.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            presentImb.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;

            presentImb.newLayout = vk::ImageLayout::ePresentSrcKHR;
            presentImb.dstAccessMask = vk::AccessFlagBits2::eNone;
            presentImb.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            presentImb.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe;

            vk::DependencyInfo presentDi;
            presentDi.dependencyFlags = vk::DependencyFlags{};
            presentDi.imageMemoryBarrierCount = 1;
            presentDi.pImageMemoryBarriers = &presentImb;
            commandBuffer.pipelineBarrier2(presentDi);

            m_renderer->endFrame();
        }
    }

    m_context->getDevice().waitIdle();
}

auto Application::getName() -> const std::string_view { return m_name; }
auto Application::get() -> Application & { return *s_instance; }

} // namespace muon
