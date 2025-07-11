#pragma once

#include <cstddef>
#include <concepts>

namespace muon {

    template<std::integral T>
    [[nodiscard]] auto Align(T integer, size_t alignment) -> T {
        return ((integer + (alignment - 1)) & ~(alignment - 1));
    }

}
