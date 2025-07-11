#include "muon/core/arena.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon {

    TEST_CASE("create arena", "[arena]") {
        ArenaAllocator allocator{1'000};
        REQUIRE(allocator.GetSize() == 1'000);
    }

    TEST_CASE("allocate uint32_t", "[arena]") {
        ArenaAllocator allocator{1'000};
        ArenaPtr integer = allocator.Allocate<uint32_t>(2'147'483'647);

        REQUIRE(*integer == 2'147'483'647);
    }

}
