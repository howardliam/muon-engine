#pragma once

#include "muon/engine/core/application.hpp"
#include "muon/engine/core/assert.hpp"
#include "muon/engine/core/log.hpp"

extern muon::Application *muon::createApplication(Application::CommandLineArgs args);

int main(int argc, char **argv) {
    muon::Log::init();

    auto app = muon::createApplication({ argc, argv });
    MU_CORE_ASSERT(app, "application must exist");

    app->run();

    delete app;
}
