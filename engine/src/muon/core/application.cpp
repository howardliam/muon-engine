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

Application::Application(const Spec &spec) {
    core::expect(!s_instance, "application already exists");
    s_instance = this;

    auto result = project::Project::Load(spec.workingDirectory / "test-project");
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
    profiling::Profiler::CreateContext(profilerSpec);

    graphics::Renderer::Spec rendererSpec{};
    rendererSpec.window = m_window.get();
    rendererSpec.context = m_context.get();
    m_renderer = std::make_unique<graphics::Renderer>(rendererSpec);

    asset::Manager::Spec assetManagerSpec{};
    assetManagerSpec.context = m_context.get();
    assetManagerSpec.loaders = {new asset::PngLoader()};
    m_assetManager = std::make_unique<asset::Manager>(assetManagerSpec);

    m_onWindowClose = m_dispatcher->Subscribe<event::WindowCloseEvent>([&](const auto &event) {
        core::info("window closed received");
        m_running = false;
    });

    m_dispatcher->Subscribe<event::WindowResizeEvent>([&](const auto &event) {
        m_context->GetGraphicsQueue().Get().waitIdle();
        m_renderer->RebuildSwapchain();
    });

    m_dispatcher->Subscribe<event::MouseButtonEvent>([](const auto &event) {
        if (event.inputState == input::InputState::Pressed && event.button == input::MouseButton::Left) {
            core::info("hello!");
        }
    });

    m_dispatcher->Subscribe<event::KeyEvent>([&](const auto &event) {
        if (event.keycode == input::KeyCode::V && event.mods.IsCtrlDown()) {
            core::info("{}", m_window->GetClipboardContents());
        }
    });

    m_dispatcher->Subscribe<event::FileDropEvent>([](const auto &event) { core::info("{}", fmt::join(event.paths, ", ")); });

    core::debug("created application");
}

Application::~Application() {
    profiling::Profiler::DestroyContext();
    core::debug("destroyed application");
}

auto Application::Get() -> Application & { return *s_instance; }

auto Application::Run() -> void {
    core::info("running application");

    graphics::ShaderCompiler::Spec compilerSpec{};
    compilerSpec.hashStorePath = project::Project::GetActiveProject()->GetProjectDirectory() / "hash_store.db";
    graphics::ShaderCompiler compiler{compilerSpec};
    compiler.SubmitWork({project::Project::GetActiveProject()->GetProjectDirectory() / "shaders/test.vert"});

    while (m_running) {
        m_window->PollEvents();

        if (auto cmd = m_renderer->BeginFrame(); cmd) {
            auto &commandBuffer = **cmd;

            m_renderer->EndFrame();
        }
    }

    m_context->GetDevice().waitIdle();
}

} // namespace muon
