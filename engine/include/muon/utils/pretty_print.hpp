#pragma once

#include <cstdint>
#include <string>

namespace muon::pp {

    [[nodiscard]] auto PrintBytes(uint64_t byteCount) -> std::string;

}
