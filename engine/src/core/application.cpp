#include "muon/core/application.hpp"

#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/renderer/device.hpp"

namespace muon {

    Application::Application(const Specification &spec) {
        MU_CORE_INFO("creating application");
        m_window = std::make_unique<Window>(Window::Properties{ spec.name, 1600, 900 });

        m_device = std::make_unique<Device>(*m_window);
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
    }

    void Application::run() {
        MU_CORE_INFO("running application");

        while (m_running) {

        }
    }

}
