#pragma once

namespace muon {

#ifdef MU_DEBUG
constexpr bool debug_enabled = true;
#else
constexpr bool debug_enabled = false;
#endif

} // namespace muon
