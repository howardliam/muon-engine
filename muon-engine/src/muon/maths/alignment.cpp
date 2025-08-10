#include "muon/maths/alignment.hpp"

namespace muon::maths {

auto align(uint64_t integer, uint64_t alignment) -> uint64_t {
    return ((integer + alignment - 1) / alignment) * alignment;
}

} // namespace muon::math
