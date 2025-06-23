#include "muon/core/application.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/debug/profiler.hpp"
#include "muon/event/event.hpp"
#include "muon/graphics/device_context.hpp"
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

        WindowSpecification windowSpec{};
        windowSpec.title = spec.name;
        windowSpec.dispatcher = &m_dispatcher;

        try {
            YAML::Node config = YAML::LoadFile("Muon.yaml");
            windowSpec.width = config["window"]["dimensions"]["width"].as<uint32_t>();
            windowSpec.height = config["window"]["dimensions"]["height"].as<uint32_t>();
        } catch (const std::exception &e) {
            MU_CORE_ERROR("{}, using default values", e.what());
            // set to rubbish so the window class knows the dimensions are bad
            windowSpec.width = std::numeric_limits<uint32_t>().max();
            windowSpec.height = std::numeric_limits<uint32_t>().max();
        }

        m_window = std::make_unique<Window>(windowSpec);

        gfx::DeviceContextSpecification deviceContextSpec{};
        deviceContextSpec.window = m_window.get();
        m_deviceContext = std::make_unique<gfx::DeviceContext>(deviceContextSpec);

        ProfilerSpecification profilerSpec{};
        profilerSpec.deviceContext = m_deviceContext.get();
        Profiler::CreateContext(profilerSpec);

        gfx::FrameManagerSpecification frameManagerSpec{};
        frameManagerSpec.window = m_window.get();
        frameManagerSpec.deviceContext = m_deviceContext.get();
        m_frameManager = std::make_unique<gfx::FrameManager>(frameManagerSpec);

        m_scriptManager = std::make_unique<ScriptManager>();

        m_dispatcher.Subscribe<event::WindowCloseEvent>([&](const auto &event) {
            MU_CORE_INFO("window closed received");
            m_running = false;
        });

        m_dispatcher.Subscribe<event::MouseButtonEvent>([&](const auto &event) {
            if (event.action == GLFW_PRESS) {
                m_scriptManager->Run();
            }
        });

        m_dispatcher.Subscribe<event::WindowFocusEvent>([&](const auto &event) {
            if (!event.focused) {
                m_window->RequestAttention();
            }
        });

        m_dispatcher.Subscribe<event::FileDropEvent>([](const auto &event) {
            MU_CORE_INFO("{}", event.paths);
        });
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
        Profiler::DestroyContext();
    }

    Window &Application::GetWindow() const {
        return *m_window;
    }

    gfx::DeviceContext &Application::GetDeviceContext() const {
        return *m_deviceContext;
    }

    Application &Application::Get() {
        return *s_instance;
    }

    void Application::Run() {
        MU_CORE_INFO("running application");

        while (m_running) {
            m_window->PollEvents();

            if (auto cmd = m_frameManager->BeginFrame()) {


                m_frameManager->EndFrame();
            }
        }

        vkDeviceWaitIdle(m_deviceContext->GetDevice());
    }

}
