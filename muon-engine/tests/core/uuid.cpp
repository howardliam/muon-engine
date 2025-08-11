#include "muon/core/uuid.hpp"

#include "catch2/catch_test_macros.hpp"

namespace muon {

TEST_CASE("default uuid is nil", "[uuid]") {
    Uuid uuid;
    REQUIRE(uuid.is_nil());
}

TEST_CASE("v4 uuid adheres to spec", "[uuid]") {
    Uuid uuid = Uuid::uuid4();
    REQUIRE(uuid.version() == UuidVersion::RandomNumber);
}

TEST_CASE("v7 uuid adheres to spec", "[uuid]") {
    Uuid uuid = Uuid::uuid7();
    REQUIRE(uuid.version() == UuidVersion::UnixTime);
}

} // namespace muon
