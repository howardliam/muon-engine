#include "muon/core/platform.hpp"

#include <csignal>
#include <unistd.h>

namespace muon {

auto invokeDebugTrap() -> void { raise(SIGTRAP); }

auto determineProcessElevation() -> bool { return geteuid() == 0; }

} // namespace muon
