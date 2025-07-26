#include "muon/core/signals.hpp"

#include <windows.h>

namespace muon {

auto debugBreak() -> void { __debugbreak(); }

} // namespace muon
