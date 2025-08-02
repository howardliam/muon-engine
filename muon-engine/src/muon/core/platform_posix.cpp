#include "muon/core/platform.hpp"

#include <csignal>
#include <unistd.h>

namespace muon {

void invokeDebugTrap() { raise(SIGTRAP); }

auto isRunAsRoot() -> bool { return geteuid() == 0; }

} // namespace muon
