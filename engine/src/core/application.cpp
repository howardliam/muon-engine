#include "muon/core/application.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/graphics/device_context.hpp"
#include "muon/graphics/mesh.hpp"
#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/mouse.hpp"
#include "muon/profiling/profiler.hpp"
#include <cstring>
#include <fmt/ranges.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <magic_enum/magic_enum.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <yaml-cpp/yaml.h>

namespace muon {

    Application::Application(const Spec &spec) {
        MU_CORE_ASSERT(!s_instance, "application already exists");
        s_instance = this;

        MU_CORE_INFO("creating application");

        m_dispatcher = std::make_unique<event::Dispatcher>();

        Window::Spec windowSpec{};
        try {
            YAML::Node config = YAML::LoadFile("Muon.yaml");
            windowSpec = config["window"].as<Window::Spec>();
        } catch (const std::exception &e) {
            MU_CORE_ERROR("{}, using default values", e.what());
        }
        windowSpec.title = spec.name;
        windowSpec.dispatcher = m_dispatcher.get();
        m_window = std::make_unique<Window>(windowSpec);

        graphics::DeviceContext::Spec deviceContextSpec{};
        deviceContextSpec.window = m_window.get();
        m_deviceContext = std::make_unique<graphics::DeviceContext>(deviceContextSpec);

        profiling::Profiler::Spec profilerSpec{};
        profilerSpec.deviceContext = m_deviceContext.get();
        profiling::Profiler::CreateContext(profilerSpec);

        graphics::Renderer::Spec rendererSpec{};
        rendererSpec.window = m_window.get();
        rendererSpec.device = m_deviceContext.get();
        m_renderer = std::make_unique<graphics::Renderer>(rendererSpec);

        m_scriptManager = std::make_unique<ScriptManager>();

        m_onWindowClose = m_dispatcher->Subscribe<event::WindowCloseEvent>([&](const auto &event) {
            MU_CORE_INFO("window closed received");
            m_running = false;
        });

        auto _ = m_dispatcher->Subscribe<event::WindowResizeEvent>([&](const auto &event) {
            vkQueueWaitIdle(m_deviceContext->GetGraphicsQueue().Get());
            m_renderer->RebuildSwapchain();
        });

        _ = m_dispatcher->Subscribe<event::MouseButtonEvent>([&](const auto &event) {
            if (event.inputState == input::InputState::Pressed && event.button == input::MouseButton::Left) {
                m_scriptManager->Run();
            }
        });

        _ = m_dispatcher->Subscribe<event::KeyEvent>([&](const auto &event) {
            if (event.keycode == input::KeyCode::V && event.mods.IsCtrlDown()) {
                MU_CORE_INFO("{}", m_window->GetClipboardContents());
            }
        });

        _ = m_dispatcher->Subscribe<event::FileDropEvent>([](const auto &event) {
            MU_CORE_INFO("{}", fmt::join(event.paths, ", "));
        });
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
        profiling::Profiler::DestroyContext();
    }

    auto Application::Get() -> Application & {
        return *s_instance;
    }

    auto Application::Run() -> void {
        MU_CORE_INFO("running application");

        struct Vertex {
            glm::vec3 position;
        };

        std::vector<Vertex> vertices{
            {{0.0, 0.5, 0.0}},
            {{0.5, -0.5, 0.0}},
            {{-0.5, -0.5, 0.0}},
        };
        std::vector<uint8_t> vertexData(vertices.size() * sizeof(Vertex));
        std::memcpy(vertexData.data(), vertices.data(), vertexData.size());

        std::vector<uint32_t> indices{
            0, 1, 2
        };

        auto cmd = m_deviceContext->GetTransferQueue().BeginCommands();

        graphics::Mesh::Spec meshSpec{};
        meshSpec.device = m_deviceContext.get();
        meshSpec.vertexData = &vertexData;
        meshSpec.vertexStride = sizeof(Vertex);
        meshSpec.indices = &indices;
        meshSpec.cmd = cmd;

        graphics::Mesh mesh{meshSpec};

        m_deviceContext->GetTransferQueue().EndCommands(cmd);

        while (m_running) {
            m_window->PollEvents();

            if (auto cmd = m_renderer->BeginFrame()) {
                m_renderer->EndFrame();
            }
        }

        vkDeviceWaitIdle(m_deviceContext->GetDevice());
    }

}
