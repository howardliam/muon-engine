#include "muon/core/application.hpp"

#include "muon/asset/loaders/png.hpp"
#include "muon/asset/manager.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/core/project.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/graphics/shader_compiler.hpp"
#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/mouse.hpp"
#include "muon/profiling/profiler.hpp"

#include <fmt/ranges.h>
#include <memory>

namespace muon {

Application::Application(const Spec &spec) {
    MU_CORE_ASSERT(!s_instance, "application already exists");
    s_instance = this;

    auto result = Project::Load(spec.workingDirectory / "test-project");
    if (!result.has_value()) {
        switch (result.error()) {
            case ProjectError::FailedToCreateDirectory:
                MU_CORE_ERROR("failed to create directory");
                break;

            case ProjectError::PathIsNotDirectory:
                MU_CORE_ERROR("path is not directory");
                break;

            case ProjectError::DirectoryIsNotEmpty:
                MU_CORE_ERROR("directory is not empty");
                break;

            case ProjectError::FailedToOpenProjectFile:
                MU_CORE_ERROR("failed to open project file");
                break;

            case ProjectError::ProjectFileDoesNotExist:
                MU_CORE_ERROR("project file does not exist");
                break;

            case ProjectError::MalformedProjectFile:
                MU_CORE_ERROR("malformed project file");
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
        MU_CORE_INFO("window closed received");
        m_running = false;
    });

    m_dispatcher->Subscribe<event::WindowResizeEvent>([&](const auto &event) {
        m_context->GetGraphicsQueue().Get().waitIdle();
        m_renderer->RebuildSwapchain();
    });

    m_dispatcher->Subscribe<event::MouseButtonEvent>([](const auto &event) {
        if (event.inputState == input::InputState::Pressed && event.button == input::MouseButton::Left) {
            MU_CORE_INFO("hello!");
        }
    });

    m_dispatcher->Subscribe<event::KeyEvent>([&](const auto &event) {
        if (event.keycode == input::KeyCode::V && event.mods.IsCtrlDown()) {
            MU_CORE_INFO("{}", m_window->GetClipboardContents());
        }
    });

    m_dispatcher->Subscribe<event::FileDropEvent>([](const auto &event) { MU_CORE_INFO("{}", fmt::join(event.paths, ", ")); });

    MU_CORE_DEBUG("created application");
}

Application::~Application() {
    profiling::Profiler::DestroyContext();
    MU_CORE_DEBUG("destroyed application");
}

auto Application::Get() -> Application & { return *s_instance; }

auto Application::Run() -> void {
    MU_CORE_INFO("running application");

    graphics::ShaderCompiler::Spec compilerSpec{};
    compilerSpec.hashStorePath = Project::GetActiveProject()->GetProjectDirectory() / "hash_store.db";
    graphics::ShaderCompiler compiler{compilerSpec};
    compiler.SubmitWork({Project::GetActiveProject()->GetProjectDirectory() / "shaders/test.vert"});

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
