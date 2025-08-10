#pragma once

namespace muon {

#ifdef MU_DEBUG

constexpr bool k_debugEnabled = true;
constexpr bool debug_enabled = true;

#else

constexpr bool k_debugEnabled = false;
constexpr bool debug_enabled = false;

#endif

} // namespace muon
