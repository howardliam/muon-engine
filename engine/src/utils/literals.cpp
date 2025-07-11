#include "muon/utils/literals.hpp"

namespace muon::literals {

    auto operator""_kb(unsigned long long value) -> size_t {
        return value * 1'000;
    }

    auto operator""_mb(unsigned long long value) -> size_t {
        return value * 1'000'000;
    }

    auto operator""_gb(unsigned long long value) -> size_t {
        return value * 1'000'000'000;
    }

}
