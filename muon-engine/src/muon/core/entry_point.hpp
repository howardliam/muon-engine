#pragma once

#include "muon/core/application.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

extern auto muon::createApplication(const std::vector<const char *> &args) -> Application *;

auto main(int32_t argc, char **argv) -> int32_t {
    muon::Log::init();

    auto app = muon::createApplication({argv, argv + argc});
    muon::core::expect(app, "application must exist");

    app->run();

    delete app;
}
