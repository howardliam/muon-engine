#include "muon/utils/literals.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::literals {

    TEST_CASE("kilobyte literal", "[literals]") {
        size_t kb = 1_kb;
        REQUIRE(kb == 1'000);
    }

    TEST_CASE("megabyte literal", "[literals]") {
        size_t mb = 1_mb;
        REQUIRE(mb == 1'000'000);
    }

    TEST_CASE("gigabyte literal", "[literals]") {
        size_t gb = 1_gb;
        REQUIRE(gb == 1'000'000'000);
    }

}
