#pragma once

#include <cstdint>
#include <string>

namespace muon::format {

auto formatBytes(uint64_t byteCount) -> std::string;

} // namespace muon::format
