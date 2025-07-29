#include "muon/core/application.hpp"

#include "muon/asset/loaders/png.hpp"
#include "muon/asset/manager.hpp"
#include "muon/config/application.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/project/project.hpp"
#include "spdlog/common.h"
#include "vulkan/vulkan_enums.hpp"

#include <exception>
#include <fstream>
#include <memory>

namespace muon {

Application::Application(const Spec &spec) : m_name{spec.name} {
    core::expect(!s_instance, "application already exists");
    s_instance = this;

    m_debugMode = spec.argParser["--debug"] == true;

    auto logLevel = m_debugMode ? spdlog::level::trace : spdlog::level::info;
    Log::setLogLevel(logLevel);

    auto result = project::Project::load(spec.workingDirectory / "test-project");
    if (!result) {
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

    auto config = loadConfig(project->getProjectDirectory() / (m_name + ".toml"));

    m_dispatcher = std::make_unique<event::Dispatcher>();

    Window::Spec windowSpec{*m_dispatcher};
    windowSpec.title = m_name;
    if (config) {
        windowSpec.width = config->width;
        windowSpec.height = config->height;
    }
    m_window = std::make_unique<Window>(windowSpec);

    graphics::Context::Spec contextSpec{*m_window};
    contextSpec.debug = m_debugMode;
    m_context = std::make_unique<graphics::Context>(contextSpec);

    graphics::Renderer::Spec rendererSpec{*m_window, *m_context};
    if (config) {
        rendererSpec.vsync = config->vsync;
    }
    m_renderer = std::make_unique<graphics::Renderer>(rendererSpec);

    asset::Manager::Spec assetManagerSpec{*m_context};
    assetManagerSpec.loaders = {new asset::PngLoader()};
    m_assetManager = std::make_unique<asset::Manager>(assetManagerSpec);

    m_onWindowClose = m_dispatcher->subscribe<event::WindowCloseEvent>([&](const auto &event) { m_running = false; });

    core::debug("created application");
}

Application::~Application() {
    auto success = writeConfig(project::Project::getActiveProject()->getProjectDirectory() / (m_name + ".toml"));
    if (!success) {
        core::error("failed to write application config");
    }

    core::debug("destroyed application");
}

auto Application::run() -> void {
    core::expect(!m_running, "application cannot already be running");
    m_running = true;
    core::info("running application");

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

auto Application::loadConfig(const std::filesystem::path &configPath) const -> std::optional<config::Application> {
    std::optional<config::Application> config;

    std::ifstream file{configPath};
    if (file.is_open()) {
        try {
            auto configToml = toml::parse(file);

            auto result = config::Application::deserialize(configToml);
            if (result) {
                config.emplace(*result);
            }
        } catch (const std::exception &e) { core::error("failed to parse config file: {}", e.what()); }
    }

    return config;
}

auto Application::writeConfig(const std::filesystem::path &configPath) const -> bool {
    config::Application config;

    config.width = m_window->getWidth();
    config.height = m_window->getHeight();
    config.vsync = m_renderer->getActivePresentMode() == vk::PresentModeKHR::eFifo;
    config.windowMode = WindowMode::Windowed;

    auto result = config::Application::serialize(config);
    if (result) {
        std::ofstream configFile{configPath, std::ios::trunc};
        configFile << *result << std::endl;

        return true;
    }

    return false;
}

} // namespace muon
