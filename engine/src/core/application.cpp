#include "muon/core/application.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/debug/profiler.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/graphics/device_context.hpp"
#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/mouse.hpp"
#include <fmt/ranges.h>
#include <GLFW/glfw3.h>
#include <limits>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <yaml-cpp/yaml.h>

namespace muon {

    Application::Application(const ApplicationSpecification &spec) {
        MU_CORE_ASSERT(!s_instance, "application already exists");
        s_instance = this;

        MU_CORE_INFO("creating application");

        m_dispatcher = std::make_unique<event::Dispatcher>();

        WindowSpecification windowSpec{};
        try {
            YAML::Node config = YAML::LoadFile("Muon.yaml");
            windowSpec = config["window"].as<WindowSpecification>();
        } catch (const std::exception &e) {
            MU_CORE_ERROR("{}, using default values", e.what());
            // set to rubbish so the window class knows the dimensions are bad
            windowSpec.width = std::numeric_limits<uint32_t>().max();
            windowSpec.height = std::numeric_limits<uint32_t>().max();
        }
        windowSpec.title = spec.name;
        windowSpec.dispatcher = m_dispatcher.get();
        m_window = std::make_unique<Window>(windowSpec);

        graphics::DeviceContextSpecification deviceContextSpec{};
        deviceContextSpec.window = m_window.get();
        m_deviceContext = std::make_unique<graphics::DeviceContext>(deviceContextSpec);

        debug::ProfilerSpecification profilerSpec{};
        profilerSpec.deviceContext = m_deviceContext.get();
        debug::Profiler::CreateContext(profilerSpec);

        graphics::RendererSpecification rendererSpec{};
        rendererSpec.window = m_window.get();
        rendererSpec.deviceContext = m_deviceContext.get();
        m_renderer = std::make_unique<graphics::Renderer>(rendererSpec);

        m_scriptManager = std::make_unique<ScriptManager>();

        m_dispatcher->Subscribe<event::WindowCloseEvent>([&](const auto &event) {
            MU_CORE_INFO("window closed received");
            m_running = false;
        });

        m_dispatcher->Subscribe<event::WindowResizeEvent>([&](const auto &event) {
            vkQueueWaitIdle(m_deviceContext->GetGraphicsQueue().Get());
            m_renderer->RebuildSwapchain();
        });

        m_dispatcher->Subscribe<event::MouseButtonEvent>([&](const auto &event) {
            if (event.inputState == input::InputState::Pressed && event.button == input::MouseButton::Left) {
                m_scriptManager->Run();
            }
        });

        m_dispatcher->Subscribe<event::KeyEvent>([&](const auto &event) {
            if (event.keycode == input::KeyCode::V && event.mods.IsCtrlDown()) {
                MU_CORE_INFO("{}", m_window->GetClipboardContents());
            }
        });

        m_dispatcher->Subscribe<event::FileDropEvent>([](const auto &event) {
            MU_CORE_INFO("{}", fmt::join(event.paths, ", "));
        });
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
        debug::Profiler::DestroyContext();
    }

    Window &Application::GetWindow() const {
        return *m_window;
    }

    graphics::DeviceContext &Application::GetDeviceContext() const {
        return *m_deviceContext;
    }

    Application &Application::Get() {
        return *s_instance;
    }

    void Application::Run() {
        MU_CORE_INFO("running application");

        while (m_running) {
            m_window->PollEvents();

            if (auto cmd = m_renderer->BeginFrame()) {
                m_renderer->EndFrame();
            }
        }

        vkDeviceWaitIdle(m_deviceContext->GetDevice());
    }

}
