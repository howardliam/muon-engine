#include "muon/core/application.hpp"
#include "muon/core/log.hpp"

namespace muon {

    Application::Application(const Specification &spec) {
        MU_CORE_INFO("creating application");
    }

    Application::~Application() {
        MU_CORE_INFO("destroying application");
    }

    void Application::run() {
        MU_CORE_INFO("running application");
    }

}
