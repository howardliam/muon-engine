#include "muon/core/application.hpp"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <yaml-cpp/yaml.h>
#include "muon/core/assert.hpp"
#include "muon/core/event/data.hpp"
#include "muon/core/input.hpp"
#include "muon/core/log.hpp"

namespace muon {

    Application::Application(const ApplicationSpecification &spec) {
        MU_CORE_ASSERT(!s_instance, "application already exists");
        s_instance = this;

        MU_CORE_INFO("creating application");

        YAML::Node config = YAML::LoadFile("Muon.yaml");

        WindowProperties properties{
            .title = spec.name,
            .width = config["window"]["dimensions"]["width"].as<uint32_t>(),
            .height = config["window"]["dimensions"]["height"].as<uint32_t>()
        };

        m_window = std::make_unique<Window>(properties, &m_dispatcher);
        m_graphicsContext = std::make_unique<gfx::Context>();

        m_scriptManager = std::make_unique<ScriptManager>();

        m_dispatcher.appendListener(EventType::WindowClose, [&](const Event &event) {
            MU_CORE_INFO("window closed received");
            m_running = false;
        });

        m_dispatcher.appendListener(EventType::MouseButton, [&](const Event &event) {
            auto data = event.get<MouseButtonEventData>();

            if (data.action == Action::Press) {
                m_scriptManager->run();
            }
        });

        m_dispatcher.appendListener(EventType::Key, [&](const Event &event) {
            auto data = event.get<KeyEventData>();

            if (data.action != Action::Press) { return; }
            if (data.mods & (GLFW_MOD_CONTROL) && data.key == GLFW_KEY_V) {
                MU_CORE_INFO(m_window->ClipboardContents());
            }
        });
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
    }

    Window &Application::GetWindow() {
        return *m_window;
    }

    gfx::Context &Application::GetGraphicsContext() {
        return *m_graphicsContext;
    }

    Application &Application::Get() {
        return *s_instance;
    }

    void Application::run() {
        MU_CORE_INFO("running application");

        // m_frameHandler->beginFrameTiming();
        // while (m_running) {
        //     m_window->pollEvents();

        //     auto cmd = m_frameHandler->beginFrame();




        //     m_frameHandler->prepareToPresent();

        //     m_frameHandler->endFrame();
        //     m_frameHandler->updateFrameTiming();
        // }

        vkDeviceWaitIdle(m_graphicsContext->GetDevice());
    }

}
