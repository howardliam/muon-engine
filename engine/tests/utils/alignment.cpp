#include "muon/utils/alignment.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon {
    TEST_CASE("align 0 by 8", "[alignment]") {
        uint32_t value = 0;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 0);
    }

    TEST_CASE("align 1 by 8", "[alignment]") {
        uint32_t value = 1;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 8);
    }

    TEST_CASE("align 2 by 8", "[alignment]") {
        uint32_t value = 2;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 8);
    }

    TEST_CASE("align 3 by 8", "[alignment]") {
        uint32_t value = 3;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 8);
    }

    TEST_CASE("align 4 by 8", "[alignment]") {
        uint32_t value = 4;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 8);
    }

    TEST_CASE("align 5 by 8", "[alignment]") {
        uint32_t value = 5;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 8);
    }

    TEST_CASE("align 6 by 8", "[alignment]") {
        uint32_t value = 6;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 8);
    }

    TEST_CASE("align 10 by 8", "[alignment]") {
        uint32_t value = 10;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 16);
    }

    TEST_CASE("align 15 by 8", "[alignment]") {
        uint32_t value = 15;
        uint32_t alignment = 8;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 16);
    }

    TEST_CASE("align 12 by 5", "[alignment]") {
        uint32_t value = 12;
        uint32_t alignment = 5;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 15);
    }

    TEST_CASE("align 3 by 7", "[alignment]") {
        uint32_t value = 3;
        uint32_t alignment = 7;
        uint32_t result = Alignment(value, alignment);
        REQUIRE(result == 7);
    }

}
