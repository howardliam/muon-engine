#pragma once

#include <cstddef>

namespace muon::literals {

    [[nodiscard]] auto operator""_kb(unsigned long long) -> size_t;
    [[nodiscard]] auto operator""_mb(unsigned long long) -> size_t;
    [[nodiscard]] auto operator""_gb(unsigned long long) -> size_t;

    [[nodiscard]] auto operator""_kib(unsigned long long) -> size_t;
    [[nodiscard]] auto operator""_mib(unsigned long long) -> size_t;
    [[nodiscard]] auto operator""_gib(unsigned long long) -> size_t;

}
