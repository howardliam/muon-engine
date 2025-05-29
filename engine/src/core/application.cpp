#include "muon/core/application.hpp"

#include "muon/core/event_data.hpp"
#include "muon/core/input.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/renderer/device.hpp"
#include "muon/renderer/frame_handler.hpp"
#include "muon/scripting/script_manager.hpp"
#include <yaml-cpp/yaml.h>

namespace muon {

    Application::Application(const ApplicationSpecification &spec) {
        MU_CORE_INFO("creating application");

        YAML::Node config = YAML::LoadFile("Muon.yaml");

        WindowProperties properties{
            .title = spec.name,
            .width = config["window"]["dimensions"]["width"].as<uint32_t>(),
            .height = config["window"]["dimensions"]["height"].as<uint32_t>()
        };

        m_window = std::make_unique<Window>(properties, &m_dispatcher);
        m_device = std::make_unique<Device>(m_window.get());
        m_frameHandler = std::make_unique<FrameHandler>(*m_window, *m_device);

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
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
    }

    void Application::run() {
        MU_CORE_INFO("running application");

        m_frameHandler->beginFrameTiming();
        while (m_running) {
            m_window->pollEvents();

            auto cmd = m_frameHandler->beginFrame();





            m_frameHandler->endFrame();
            m_frameHandler->updateFrameTiming();
        }

        m_device->device().waitIdle();
    }

}
