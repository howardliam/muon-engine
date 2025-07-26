#include "muon/core/signals.hpp"

#include <csignal>

namespace muon {

auto debugBreak() -> void { raise(SIGTRAP); }

} // namespace muon
