#include "muon/core/application.hpp"

#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/renderer/device.hpp"
#include "muon/renderer/frame_handler.hpp"

namespace muon {

    Application::Application(const Specification &spec) {
        MU_CORE_INFO("creating application");

        m_window = std::make_unique<Window>(Window::Properties{ spec.name, 1600, 900 }, &m_dispatcher);
        m_device = std::make_unique<Device>(*m_window);
        m_frameHandler = std::make_unique<FrameHandler>(*m_window, *m_device);

        m_dispatcher.appendListener(EventType::WindowClose, [&](const Event &event) {
            MU_CORE_INFO("window closed receive");
            const auto &test = event.get<CloseEventData>();
            m_running = false;
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
    }

}
