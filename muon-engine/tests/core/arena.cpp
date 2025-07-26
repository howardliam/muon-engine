#include "muon/core/arena.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon {

TEST_CASE("allocate uint32_t", "[arena]") {
    ArenaAllocator allocator{1000};
    ArenaPtr data = allocator.Create<uint32_t>(2'147'483'647);

    REQUIRE(*data == 2'147'483'647);
}

TEST_CASE("allocate custom struct", "[arena]") {
    ArenaAllocator allocator{1000};

    struct TestStruct {
        uint32_t foo{0};
        bool bar{false};
    };

    ArenaPtr data = allocator.Create<TestStruct>(0, false);

    REQUIRE(data->foo == 0);
    REQUIRE(data->bar == false);
}

TEST_CASE("allocate custom struct with constructor", "[arena]") {
    ArenaAllocator allocator{1000};

    struct TestStruct {
        uint32_t foo;
        TestStruct() : foo{5} {}
    };

    ArenaPtr data = allocator.Create<TestStruct>();

    REQUIRE(data->foo == 5);
}

} // namespace muon
