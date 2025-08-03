#pragma once

#include "muon/core/application.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/crypto/crypto.hpp"

#include <cstdlib>

extern auto muon::createApplication(size_t argCount, char **argArray) -> Application *;

auto main(int32_t argCount, char **argArray) -> int32_t {
    muon::Log::init();

    muon::crypto::init();

    if (muon::isRunAsRoot()) {
        muon::core::error("cannot be run with elevated privileges");
        std::exit(1);
    }

    auto app = muon::createApplication(argCount, argArray);
    muon::core::expect(app, "application must exist");

    app->run();

    delete app;

    muon::crypto::cleanup();
}
