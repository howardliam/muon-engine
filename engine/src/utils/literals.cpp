#include "muon/utils/literals.hpp"

namespace muon::literals {

    auto operator""_kb(unsigned long long value) -> size_t {
        return value * 1000;
    }

    auto operator""_mb(unsigned long long value) -> size_t {
        return value * 1000 * 1000;
    }

    auto operator""_gb(unsigned long long value) -> size_t {
        return value * 1000 * 1000 * 1000;
    }

    auto operator""_kib(unsigned long long value) -> size_t {
        return value * 1024;
    }

    auto operator""_mib(unsigned long long value) -> size_t {
        return value * 1024 * 1024;
    }

    auto operator""_gib(unsigned long long value) -> size_t {
        return value * 1024 * 1024 * 1024;
    }

}
