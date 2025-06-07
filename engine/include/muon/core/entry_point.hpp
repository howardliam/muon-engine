#pragma once

#include "muon/core/application.hpp"
#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"

extern muon::Application *muon::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char **argv) {
    muon::Log::Init();

    auto app = muon::CreateApplication({ argc, argv });
    MU_CORE_ASSERT(app, "application must exist");

    app->Run();

    delete app;
}
