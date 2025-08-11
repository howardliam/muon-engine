#pragma once

namespace muon {

#ifdef MU_DEBUG
constexpr bool DEBUG_ENABLED = true;
#else
constexpr bool DEBUG_ENABLED = false;
#endif

} // namespace muon
