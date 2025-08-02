#pragma once

#include <concepts>
#include <cstddef>

namespace muon::utils {

template <std::integral T>
auto alignment(T integer, size_t alignment) -> T {
    return ((integer + alignment - 1) / alignment) * alignment;
}

} // namespace muon
