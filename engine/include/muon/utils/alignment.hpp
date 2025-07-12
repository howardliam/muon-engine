#pragma once

#include <concepts>
#include <cstddef>

namespace muon {

template <std::integral T>
[[nodiscard]] auto Alignment(T integer, size_t alignment) -> T {
    return ((integer + alignment - 1) / alignment) * alignment;
}

} // namespace muon
