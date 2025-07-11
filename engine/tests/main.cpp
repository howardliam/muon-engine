#include "muon/core/log.hpp"

#include <cstdint>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>

auto main(int32_t count, char **arguments) -> int32_t {
    muon::Log::Init();

    return Catch::Session().run(count, arguments);
}
