#pragma once

namespace muon {

#ifdef MU_DEBUG

constexpr bool k_debugEnabled = true;

#else

constexpr bool k_debugEnabled = false;

#endif

} // namespace muon
