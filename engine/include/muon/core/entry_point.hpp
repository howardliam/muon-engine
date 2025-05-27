#pragma once

#include "muon/core/application.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

extern muon::Application *muon::createApplication(Application::CommandLineArgs args);

int main(int argc, char **argv) {
    muon::Log::init();

    auto app = muon::createApplication({ argc, argv });
    MU_CORE_ASSERT(app, "application must exist");

    app->run();

    delete app;
}
