#include "muon/core/uuid.hpp"

#include "catch2/catch_test_macros.hpp"

namespace muon {

TEST_CASE("default uuid is nil", "[uuid]") {
    uuid uuid;
    REQUIRE(uuid.is_nil());
}

TEST_CASE("v4 uuid adheres to spec", "[uuid]") {
    uuid4_generator gen;
    uuid uuid = gen();
    REQUIRE(uuid.version() == uuid::version_random_number_based);
}

TEST_CASE("v7 uuid adheres to spec", "[uuid]") {
    uuid7_generator gen;
    uuid uuid = gen();
    REQUIRE(uuid.version() == uuid::version_unix_time_based);
}

} // namespace muon
