#pragma once

#include "muon/core/application.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

#include <string>

extern auto muon::createApplication(const std::vector<std::string> &args) -> Application *;

auto main(int32_t argCount, char **argArray) -> int32_t {
    muon::Log::init();

    std::vector<std::string> args = {argArray, argArray + argCount};
    auto app = muon::createApplication(args);
    muon::core::expect(app, "application must exist");

    app->run();

    delete app;
}
