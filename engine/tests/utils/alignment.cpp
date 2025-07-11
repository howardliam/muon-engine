#include "muon/utils/alignment.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon {

    TEST_CASE("align 10 by 8", "[alignment]") {
        uint32_t result = Align(10, 8);
        REQUIRE(result == 16);
    }

}
