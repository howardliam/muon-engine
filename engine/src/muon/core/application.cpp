#include "muon/core/application.hpp"

#include "muon/asset/loaders/png.hpp"
#include "muon/asset/manager.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/graphics/shader_compiler.hpp"
#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/mouse.hpp"
#include "muon/profiling/profiler.hpp"
#include "muon/project/project.hpp"

#include <fmt/ranges.h>
#include <memory>

namespace muon {

Application::Application(const Spec &spec) : m_name{spec.name} {
    core::expect(!s_instance, "application already exists");
    s_instance = this;

    auto result = project::Project::load(spec.workingDirectory / "test-project");
    if (!result.has_value()) {
        switch (result.error()) {
            case project::ProjectError::FailedToCreateDirectory:
                core::error("failed to create directory");
                break;

            case project::ProjectError::PathIsNotDirectory:
                core::error("path is not directory");
                break;

            case project::ProjectError::DirectoryIsNotEmpty:
                core::error("directory is not empty");
                break;

            case project::ProjectError::FailedToOpenProjectFile:
                core::error("failed to open project file");
                break;

            case project::ProjectError::ProjectFileDoesNotExist:
                core::error("project file does not exist");
                break;

            case project::ProjectError::MalformedProjectFile:
                core::error("malformed project file");
                break;
        }
    }
    std::shared_ptr project = *result;

    m_dispatcher = std::make_unique<event::Dispatcher>();

    Window::Spec windowSpec{};
    windowSpec.title = spec.name;
    windowSpec.dispatcher = m_dispatcher.get();
    m_window = std::make_unique<Window>(windowSpec);

    graphics::Context::Spec contextSpec{};
    contextSpec.window = m_window.get();
    contextSpec.debug = true;
    m_context = std::make_unique<graphics::Context>(contextSpec);

    profiling::Profiler::Spec profilerSpec{};
    profilerSpec.context = m_context.get();
    profiling::Profiler::createContext(profilerSpec);

    graphics::Renderer::Spec rendererSpec{};
    rendererSpec.window = m_window.get();
    rendererSpec.context = m_context.get();
    m_renderer = std::make_unique<graphics::Renderer>(rendererSpec);

    asset::Manager::Spec assetManagerSpec{};
    assetManagerSpec.context = m_context.get();
    assetManagerSpec.loaders = {new asset::PngLoader()};
    m_assetManager = std::make_unique<asset::Manager>(assetManagerSpec);

    m_onWindowClose = m_dispatcher->subscribe<event::WindowCloseEvent>([&](const auto &event) {
        core::info("window closed received");
        m_running = false;
    });

    m_dispatcher->subscribe<event::WindowResizeEvent>([&](const auto &event) {
        m_context->getGraphicsQueue().get().waitIdle();
        m_renderer->rebuildSwapchain();
    });

    m_dispatcher->subscribe<event::MouseButtonEvent>([](const auto &event) {
        if (event.inputState == input::InputState::Pressed && event.button == input::MouseButton::Left) {
            core::info("hello!");
        }
    });

    m_dispatcher->subscribe<event::KeyEvent>([&](const auto &event) {
        if (event.keycode == input::KeyCode::V && event.mods.isCtrlDown()) {
            core::info("{}", m_window->getClipboardContents());
        }
    });

    m_dispatcher->subscribe<event::FileDropEvent>([](const auto &event) { core::info("{}", fmt::join(event.paths, ", ")); });

    core::debug("created application");
}

Application::~Application() {
    profiling::Profiler::destroyContext();
    core::debug("destroyed application");
}

auto Application::getName() -> const std::string_view { return m_name; }

auto Application::get() -> Application & { return *s_instance; }

auto Application::run() -> void {
    core::info("running application");

    graphics::ShaderCompiler::Spec compilerSpec{};
    compilerSpec.hashStorePath = project::Project::getActiveProject()->getProjectDirectory() / "hash_store.db";
    graphics::ShaderCompiler compiler{compilerSpec};
    compiler.submitWork({project::Project::getActiveProject()->getProjectDirectory() / "shaders/test.vert"});

    while (m_running) {
        m_window->pollEvents();

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

            vk::DependencyInfo reset;
            reset.dependencyFlags = vk::DependencyFlags{};
            reset.imageMemoryBarrierCount = 1;
            reset.pImageMemoryBarriers = &resetImb;
            commandBuffer.pipelineBarrier2(reset);

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

} // namespace muon
