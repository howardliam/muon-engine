#include "muon/utils/literals.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::literals {

TEST_CASE("kilobyte literal", "[literals]") {
    size_t kb = 1_kb;
    REQUIRE(kb == 1000);
}

TEST_CASE("megabyte literal", "[literals]") {
    size_t mb = 1_mb;
    REQUIRE(mb == 1000000);
}

TEST_CASE("gigabyte literal", "[literals]") {
    size_t gb = 1_gb;
    REQUIRE(gb == 1000000000);
}

TEST_CASE("kibibyte literal", "[literals]") {
    size_t kb = 1_kib;
    REQUIRE(kb == 1024);
}

TEST_CASE("mebibyte literal", "[literals]") {
    size_t mb = 1_mib;
    REQUIRE(mb == 1048576);
}

TEST_CASE("gibibyte literal", "[literals]") {
    size_t gb = 1_gib;
    REQUIRE(gb == 1073741824);
}

} // namespace muon::literals
