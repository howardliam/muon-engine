#include "muon/core/log.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

#include <cstdint>

auto main(int32_t count, char **arguments) -> int32_t {
    muon::log::init();

    return Catch::Session().run(count, arguments);
}
