#pragma once

#include "muon/core/application.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

extern auto muon::CreateApplication(ApplicationCommandLineArgs args) -> Application *;

auto main(int32_t argc, char **argv) -> int32_t {
    muon::Log::Init();

    auto app = muon::CreateApplication({argc, argv});
    muon::core::expect(app, "application must exist");

    app->Run();

    delete app;
}
