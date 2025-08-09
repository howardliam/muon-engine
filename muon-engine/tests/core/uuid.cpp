#include "muon/core/uuid.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon {

TEST_CASE("default uuid is nil", "[uuid]") {
    uuid uuid;
    REQUIRE(uuid.is_nil());
}

TEST_CASE("v4 uuid adheres to spec", "[uuid]") {
    uuid4_generator gen;
    uuid uuid = gen();
    REQUIRE(((uuid.data[6] & 0xf0) >> 4) == 4);
    REQUIRE(((uuid.data[8] & 0xc0) >> 6) == 0b10);
}

} // namespace muon
